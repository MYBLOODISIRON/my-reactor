#include <functional>
#include <sys/socket.h>
#include <errno.h>
#include "TcpConnection.h"
#include "Logger.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr)
:   m_loop  {},
    m_name  {name},
    m_state {kConnecting},
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
    LOG_INFO("TcpConnection::dtor[%d] at fd = %d.\n", m_name.c_str(), m_channel->fd());
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
    return m_state == kConnected;
}

void TcpConnection::send(const void* message, int len)
{

}

void TcpConnection::shutdown()
{

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

void TcpConnection::connectEstablished()
{

}

void TcpConnection::connectDestroyed()
{

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
                if(m_state == kDisconnecting)
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
    LOG_INFO("fd = %d state = %d.\n", m_channel->fd(), m_state);
    setState(kDisconnected);
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

}

void TcpConnection::shutdownInLoop()
{

}

void TcpConnection::setState(StateE state)
{
    m_state = state;
}