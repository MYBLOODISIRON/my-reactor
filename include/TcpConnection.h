#pragma once
#include <memory>
#include <any>
#include <atomic>
#include <string>
#include "noncopyable.h"
#include "Timestamp.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
class Channel;
class EventLoop;
class Socket;


class TcpConnection: noncopyable, public std::enable_shared_from_this< TcpConnection >
{
    enum ConnectionState
    {
        Connecting,
        Connected,
        Disconnecting,
        Disconnected
    };

    private:
        EventLoop   *m_loop {nullptr};
        const   std::string     m_name;
        std::atomic_int         m_state;
        bool                    m_reading;

        std::unique_ptr < Socket >  m_socket;
        std::unique_ptr < Channel > m_channel;

        const   InetAddress     m_localAddr;
        const   InetAddress     m_peerAddr;

        ConnectionCallback      m_connectionCallback;
        MessageCallback         m_messageCallback;
        WriteCompleteCallback   m_writeCompleteCallback;
        HighWaterMarkCallback   m_highWaterMarkCallback;
        CloseCallback           m_closeCallback;


        size_t      m_highWaterMark;
        Buffer      m_inputBuffer;
        Buffer      m_outputBuffer;
        std::any    m_context;

    public: 
        TcpConnection   (EventLoop *loop, const std::string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr);
        ~TcpConnection  ();

        EventLoop*  getLoop () const;
        const std::string&  name    () const;
        const InetAddress&  localAddress    () const;
        const InetAddress&  peerAddress     () const;

        bool connected  () const;
        void send       (const void* message, int len);
        void shutdown   ();

        void setConnectionCallback  (const ConnectionCallback& cb);
        void setMessageCallback     (const MessageCallback& cb);
        void setWriteCompleteCallback   (const WriteCompleteCallback& cb);
        void setHighWaterMarkCallback   (const HighWaterMarkCallback& cb);
        void setCloseCallback           (const CloseCallback& cb);

        void connectEstablished ();
        void connectDestroyed   ();
        void setState           (ConnectionState state);
        void send               (const std::string& buf);


        void    setContext  (const std::any& context);
        const std::any&  getContext  () const;

    private:
        void handleWrite    ();
        void handleRead     (Timestamp recieveTime);
        void handleClose    ();
        void handleError    ();

        void sendInLoop     (const void* message, size_t len);
        void shutdownInLoop ();
};