#include <functional>
#include <any>
#include <string>
#include <sys/socket.h>
#include <errno.h>
#include "TcpConnection.h"
#include "Logger.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr)
:   m_loop  {loop},
    m_name  {name},
    m_state {Connecting},
    m_reading   {true},
    m_socket    {new Socket {sockfd}},
    m_channel   {new Channel {loop, sockfd}},
    m_localAddr {localAddr},
    m_peerAddr  {peerAddr},
    m_highWaterMark {64 * 1024 * 1024}
{
    m_channel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    m_channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    m_channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    m_channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_INFO("TcpConnection::ctor[%s] at fd = %d.\n", name.c_str(), sockfd);
    m_socket->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd = %d.\n", m_name.c_str(), m_channel->fd());
}

EventLoop* TcpConnection::getLoop() const
{
    return m_loop;
}

const std::string& TcpConnection::name() const
{
    return m_name;
}

const InetAddress& TcpConnection::localAddress() const
{
    return m_localAddr;
}

const InetAddress& TcpConnection::peerAddress() const
{
    return m_peerAddr;
}


bool TcpConnection::connected() const
{
    return m_state == Connected;
}


void TcpConnection::shutdown()
{
    if(m_state == Connected)
    {
        setState(Disconnecting);
        m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::setConnectionCallback(const ConnectionCallback& cb)
{
    m_connectionCallback = cb;
}

void TcpConnection::setMessageCallback(const MessageCallback& cb)
{
    m_messageCallback = cb;
}

void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb)
{
    m_writeCompleteCallback = cb;
}

void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb)
{
    m_highWaterMarkCallback = cb;
}

void TcpConnection::setCloseCallback(const CloseCallback& cb)
{
    m_closeCallback = cb;
}

void TcpConnection::connectEstablished()    // 建立连接
{
    setState(Connected);
    m_channel->tie(shared_from_this());
    m_channel->enableReading();
    m_connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if(m_state == Connected)
    {
        setState(Disconnected);
        m_channel->disableAll();
        m_closeCallback(shared_from_this());
    }
    m_channel->remove();
}

void TcpConnection::handleWrite()
{
    if(m_channel->isWriting())
    {
        int savedErrno = 0;
        ssize_t n = m_outputBuffer.writeFd(m_channel->fd(), &savedErrno);
        if(n > 0)
        {
            m_outputBuffer.retrieve(n);
            if(m_outputBuffer.readableBytes() == 0)
            {
                m_channel->disableWriting();
                if(m_writeCompleteCallback)
                {
                    m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
                }
                if(m_state == Disconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite.\n");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd = %d is down, no more writing.\n", m_channel->fd());
    }
}

void TcpConnection::handleRead(Timestamp recieveTime)
{
    int savedErrno = 0;
    ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &savedErrno);
    if(n > 0)
    {
        m_messageCallback(shared_from_this(), &m_inputBuffer, recieveTime);     
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleClose()
{
    int state = m_state;
    LOG_INFO("fd = %d state = %d.\n", m_channel->fd(), state);
    setState(Disconnected);
    m_channel->disableAll();

    TcpConnectionPtr connPtr {shared_from_this()};
    m_connectionCallback(connPtr);
    m_closeCallback(connPtr);
}

void TcpConnection::handleError()
{
    int optval {0};
    socklen_t optlen {sizeof optval};
    int err {0};
    if(::getsockopt(m_channel->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d.\n", m_name.c_str(), err);
}

void TcpConnection::sendInLoop(const void* message, size_t len)
{
    ssize_t nwrote {0};
    size_t remaining {len};
    bool faultError {false};
    if(m_state == Disconnected)    // 已调用shutdown
    {
        LOG_ERROR("disconnected, give up writing.\n");
        return ;
    }
    if(! m_channel->isWriting() && m_outputBuffer.readableBytes() == 0) // channel第一次发送数据，且缓冲区没有数据
    {
        nwrote = ::write(m_channel->fd(), message, len);
        if(nwrote >= 0)
        {
            remaining = len - nwrote;
            if(remaining == 0 && m_writeCompleteCallback)
            {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if(errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop.\n");
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }
    if(! faultError && remaining > 0)   // 数据未全部发送
    {
        size_t oldLen {m_outputBuffer.readableBytes()};
        if((oldLen + remaining) >= m_highWaterMark && oldLen < m_highWaterMark && m_highWaterMarkCallback)
        {
            m_loop->queueInLoop(std::bind(m_highWaterMarkCallback, shared_from_this(), oldLen + remaining));

        }
        m_outputBuffer.append((char*)message + nwrote, remaining);
        if(!m_channel->isWriting())
        {
            m_channel->enableWriting();
        }
    }
}

void TcpConnection::shutdownInLoop()
{
    if(! m_channel->isWriting())  // outputBuffer数据全部发送完成
    {
        m_socket->shutdownWrite();
    }
}

void TcpConnection::setState(ConnectionState state)
{
    m_state = state;
}

void TcpConnection::send(const std::string& buf)
{
    if(m_state == Connected)
    {
        if(m_loop->isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            m_loop->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}



void TcpConnection::setContext(const std::any& context)
{
    m_context = context;
}

const std::any& TcpConnection::getContext() const
{
    return m_context;
}