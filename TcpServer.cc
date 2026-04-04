#include <functional>
#include "TcpServer.h"
#include "Logger.h"

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
    m_nextConnId            {1}
{
    m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2)); // 用户连接时，会执行TcpServer::newConnection回调
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


void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{

}