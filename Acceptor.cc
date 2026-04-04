#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%s listen socket create fatal: %d.\n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress& listenAddr, bool reusePort)
:   m_loop {loop},
    m_acceptSocket  {createNonblocking()},
    m_acceptChannel {loop, m_acceptSocket.fd()},
    m_listening     {false}
{
    m_acceptSocket.setReuseAddr(true);
    m_acceptSocket.setReusePort(true);
    m_acceptSocket.bindAddress(listenAddr);

    m_acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}


void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = m_acceptSocket.accept(&peerAddr);
    if(connfd >= 0)
    {
        if(m_newConnectionCallback)
        {
            m_newConnectionCallback(connfd, peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept err %d.\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if(errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d fd has reached the limit.\n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback& cb)
{
    m_newConnectionCallback = cb;
}

Acceptor::~Acceptor()
{
    m_acceptChannel.disableAll();
    m_acceptChannel.remove();
}

void Acceptor::listen()
{
    m_listening = true;
    m_acceptSocket.listen();
    m_acceptChannel.enableReading();
}

bool Acceptor::listening() const
{
    return m_listening;
}