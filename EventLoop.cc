#include <unistd.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <memory>
#include <mutex>
#include <vector>


#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include "CurrentThread.h"
#include "Poller.h"

__thread EventLoop *loopInThread = nullptr; // 防止一个线程创建多个EventLoop

const int kPollTimeMs = 10000; // 默认超时时间

int createEventfd()
{
    int evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evfd < 0)
    {
        LOG_FATAL("evfd error: %d\n", errno);
    }

    return evfd;
}



EventLoop::EventLoop()
:   m_threadId  {CurrentThread::tid()}, 
    m_poller    {Poller::newDefaultPoller(this)}, 
    m_wakeupFd  {createEventfd()}, 
    m_wakeupChannel {new Channel {this, m_wakeupFd}}
{
    if(loopInThread)
    {
        LOG_FATAL("Another EventLoop %p is exist in this thread %lu\n", loopInThread,
                  static_cast<unsigned long>(m_threadId));
    }
    else
    {
        loopInThread = this;
    }

    // 设置wakeupfd的事件类型及其发生后的回调
    m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
    m_wakeupChannel->enableReading();
}

EventLoop::~EventLoop()
{
    m_wakeupChannel->disableAll();
    m_wakeupChannel->remove();
    close(m_wakeupFd);
    loopInThread = nullptr;
}


void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(m_wakeupFd, &one, sizeof(one));
    if(n != 8)
    {
        LOG_ERROR("EventLoop::handleRead() reads %ld bytes instead of 8\n", n);
    }
}


void EventLoop::loop()
{
    m_looping = true;
    m_quit = false;

    while(! m_quit)
    {
        m_activeChannels.clear();
        m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_activeChannels);
        for(Channel* channel : m_activeChannels)
        {
            channel->handleEvent(m_pollReturnTime);
        }

        doPendingFunctor(); // 执行EventLoop事件循环需要处理的回调操作,由mainloop注册，subloop执行
    }
    m_looping = false;
}


void EventLoop::quit()
{
    m_quit = true;

    if(! isInLoopThread())  // 非loop所在的线程调用quit，会唤醒loop所在的线程，并且跳出while循环
    {
        wakeup();
    }
}


void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else    // 非loop所在的线程
    {
        queueInLoop(cb);
    }
}


void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex>    lock    {m_mutex};
        m_pendingFunctors.emplace_back(cb);

    }

    if(! isInLoopThread() || m_callingPendingFunctor)  // 唤醒loop所在的线程
    {
        wakeup();
    }
}


void EventLoop::wakeup()
{
    uint64_t one {0};
    ssize_t n {write(m_wakeupFd, &one, 8)};
    if(n != 8)
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 bytes.\n", n);
    }
}


void EventLoop::updateChannel(Channel *channel)
{
    m_poller->updateChannel(channel);
}


void EventLoop::removeChannel(Channel *channel)
{
    m_poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return m_poller->hasChannel(channel);
}


void EventLoop::doPendingFunctor()
{
    std::vector<Functor>    functors;
    m_callingPendingFunctor = true;

    {
        std::unique_lock<std::mutex>    lock    {m_mutex};
        functors.swap(m_pendingFunctors);   // 可以和注册回调同时执行
    }

    for(const auto& functor : functors)
    {
        functor();  // 执行回调
    }

    m_callingPendingFunctor = false;
}


bool EventLoop::isInLoopThread() const
{
    return pthread_equal(m_threadId, CurrentThread::tid()) != 0;
}

Timestamp EventLoop::pollReturnTime() const
{
    return m_pollReturnTime;
}