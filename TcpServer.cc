#include <functional>
#include <string.h>
#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"

EventLoop* CheckLoopNotNull(EventLoop *loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d main loop is null.\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg, Option option)
:   m_loop      {CheckLoopNotNull(loop)},
    m_ipPort    {listenAddr.toIpPort()},
    m_name      {nameArg},
    m_acceptor  {new Acceptor {loop, listenAddr, option == kReusePort}},
    m_threadPool    {new EventLoopThreadPool {loop, nameArg}},
    m_connectionCallback    {},
    m_messageCallback       {},
    m_nextConnId            {1},
    m_started               {0}
{
    m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2)); // 用户连接时，会执行TcpServer::newConnection回调
}

TcpServer::~TcpServer()
{
    for(auto& item : m_connections)
    {
        TcpConnectionPtr conn {item.second};    // 局部智能指针对象
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadInitCallback(const ThreadInitCallback& cb)
{
    m_threadInitCallback = cb;
}

void TcpServer::setConnectionCallback(const ConnectionCallback& cb)
{
    m_connectionCallback = cb;
}

void TcpServer::setMessageCallback(const MessageCallback& cb)
{
    m_messageCallback = cb;
}

void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback& cb)
{
    m_writeCompleteCallback = cb;
}


void TcpServer::setThreadNum(int numThreads)
{
    m_threadPool->setNumThreads(numThreads);
}

void TcpServer::start()
{
    if(m_started ++ == 0)   // 防止一个TcpServer被start多次
    {
        m_threadPool->start(m_threadInitCallback);
        m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
    }
}


void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)  // 新客户端连接，acceptor执行这个回调
{
    EventLoop* ioLoop = m_threadPool->getNextLoop();    // 轮询
    char buf [64];
    snprintf(buf, 64, "-%s#%d", m_ipPort.c_str(), m_nextConnId);
    m_nextConnId ++;
    std::string connName {m_name + buf};

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s.\n", m_name.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    struct sockaddr_in local;
    ::memset(&local, 0, sizeof local);
    socklen_t addrlen {sizeof local};
    if(::getsockname(sockfd, (sockaddr*) &local, &addrlen) < 0)
    {
        LOG_ERROR("sockets::getLocalAddr.\n");
    }

    InetAddress localAddr {local};
    TcpConnectionPtr conn {new TcpConnection {ioLoop, connName, sockfd, localAddr, peerAddr}};

    m_connections [connName] = conn;
    conn->setConnectionCallback(m_connectionCallback);

    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    m_loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s.\n", m_name.c_str(), conn->name().c_str());

    m_connections.erase(conn->name());
    EventLoop* ioLoop {conn->getLoop()};
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}