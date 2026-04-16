#pragma once
#include "noncopyable.h"
class InetAddress;


class Socket
{
    private:

        const int m_sockfd;

    public:

        explicit Socket (int sockfd);
        ~Socket         ();

        int fd  () const;
        void    bindAddress (const InetAddress& localAddr);
        void    listen      ();
        int     accept      (InetAddress *peerAddr);
        void    shutdownWrite   ();

        void    setTcpNoDelay   (bool on);
        void    setReuseAddr    (bool on);
        void    setReusePort    (bool on);
        void    setKeepAlive    (bool on);
};