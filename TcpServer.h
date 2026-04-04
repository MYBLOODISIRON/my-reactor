#pragma once
#include <functional>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <string>
#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"

class TcpServer: noncopyable
{
    enum Option
    {
        kNoReusePort,
        kReusePort
    };

    using ThreadInitCallback    = std::function< void(EventLoop*) >;
    using ConnectionMap         = std::unordered_map< std::string, TcpConnectionPtr >;
    private:
        EventLoop   *m_loop;
        const std::string   m_ipPort;
        const std::string   m_name;
        std::unique_ptr< Acceptor >     m_acceptor;
        std::unique_ptr< EventLoopThreadPool >  m_threadPool;

        ConnectionCallback  m_connectionCallback;   // 产生新链接的回调
        MessageCallback     m_messageCallback;      // 有读写消息的回调
        WriteCompleteCallback   m_writeCompleteCallback;    // 消息发送完成的回调

        ThreadInitCallback  m_threadInitCallback;   // 线程初始化回调
        std::atomic_int     m_started;
        int     m_nextConnId;
        ConnectionMap       m_connections;
    public:
        TcpServer   (EventLoop *loop, const InetAddress& listenAddr, Option option = kNoReusePort);
        ~TcpServer  ();

        void    setThreadInitCallback   (const ThreadInitCallback& cb);
        void    setConnectionCallback   (const ConnectionCallback& cb);
        void    setMessageCallback      (const MessageCallback& cb);
        void    setWriteCompleteCallback    (const WriteCompleteCallback& cb);

        void    setthreadNum            (int numThreads);
        void    start                   (); // 开启服务器监听

    private:
        void    newConnection   (int sockfd, const InetAddress& peerAddr);
        void    removeConnection    (const TcpConnectionPtr& conn);
        void    removeConnectionInLoop  (const TcpConnectionPtr& conn);
};