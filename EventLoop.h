#pragma once
#include <vector>
#include <functional>
#include <atomic>
#include <memory>
#include <mutex>
#include "noncopyable.h"
#include "Timestamp.h"
class Channel;
class Poller;


class EventLoop
{
    using Functor = std::function< void() >;
    using ChannelList = std::vector< Channel* >;

    private:

        std::atomic_bool    m_looping   {false};
        std::atomic_bool    m_quit      {false};     // 标志推出循环
        const   pid_t       m_threadId;     // loop所在的线程
        Timestamp           m_pollReturnTime;
        std::unique_ptr<Poller> m_poller;

        int m_wakeupFd; // mainloop获取新Channel，唤醒subloop，是一个eventfd
        std::unique_ptr<Channel>    m_wakeupChannel;

        ChannelList m_activeChannels;
        Channel*    m_currentActiveChannel  {nullptr};

        std::atomic_bool        m_callingPendingFunctor {false};    // 标识当前loop是否有需要执行的回调
        std::vector<Functor>    m_pendingFunctors;
        std::mutex              m_mutex;    // 保护m_pendingFunctors操作

    public:
        EventLoop   ();
        ~EventLoop  ();

        void loop   (); // 开启事件循环
        void quit   (); // 退出事件循环

        Timestamp   pollReturnTime  () const;

        void    runInLoop   (Functor cb);   // 当前loop执行cb
        void    queueInLoop (Functor cb);   // cb放进queue，唤醒loop所在的线程，执行cb

        void    wakeup  ();
        void    updateChannel   (Channel *channel);
        void    removeChannel   (Channel *channel);
        bool    hasChannel      (Channel *channel);

        bool     isInLoopThread () const;
    
    private:

        void handleRead ();
        void doPendingFunctor   ();
};