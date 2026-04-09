#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/tcp.h>

#include "Socket.h"
#include "Logger.h"
#include "InetAddress.h"


Socket::Socket(int sockfd)
:   m_sockfd    {sockfd}
{

}


Socket::~Socket()
{
    close(m_sockfd);
}

int Socket::fd() const
{
    return m_sockfd;
}

void Socket::bindAddress(const InetAddress& localAddr)
{
    if(0 != bind(m_sockfd, (sockaddr*)localAddr.getSockAddr(), sizeof(sockaddr_in)))
    {
        LOG_FATAL("bind sockfd: %d failed.\n", m_sockfd);
    }
}

void Socket::listen()
{
    if(0 != ::listen(m_sockfd, 1024))
    {
        LOG_FATAL("listen sockfd: %d failed.\n", m_sockfd);
    }
}


int Socket::accept(InetAddress* peerAddr)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    socklen_t len {sizeof addr};

    int connfd = ::accept4(m_sockfd, (sockaddr*) &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd >= 0)
    {
        peerAddr->setSockAddr(addr);
    }
    return connfd;
}


void Socket::shutdownWrite()
{
    if(::shutdown(m_sockfd, SHUT_WR) < 0)
    {
        LOG_ERROR("shutdown error.\n");
    }
}


void Socket::setTcpNoDelay(bool on)
{
    int optval {on ? 1 : 0};
    ::setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}


void Socket::setReuseAddr(bool on)
{
    int optval {on ? 1 : 0};
    ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::setReusePort(bool on)
{
    int optval {on ? 1 : 0};

    ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}

void Socket::setKeepAlive(bool on)
{
    int optval {on ? 1 : 0};
    ::setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}