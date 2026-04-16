#pragma once 
#include <functional>
#include "Socket.h"
#include "noncopyable.h"
#include "Channel.h"
class EventLoop;
class InetAddress;


class Acceptor: noncopyable
{
    using NewConnectionCallback = std::function< void(int, const InetAddress&) >;

    private:
        EventLoop   *m_loop         {nullptr};
        Socket      m_acceptSocket;
        Channel     m_acceptChannel;
        NewConnectionCallback   m_newConnectionCallback;
        bool                    m_listening {false};
    public:
        Acceptor    (EventLoop *loop, const InetAddress& listenAddr, bool reusePort);
        ~Acceptor   ();

        void    setNewConnectionCallback    (const NewConnectionCallback& cb);
        bool    listening                   () const;
        void    listen                      ();
    private:
        void    handleRead  ();
};