#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>

#include "Thread.h"
#include "noncopyable.h"

class EventLoop;

class EventLoopThread: noncopyable
{

    using ThreadInitCallback = std::function<void(EventLoop*)>;

    private:

        EventLoop   *m_loop {nullptr};
        bool        m_exiting   {false};
        Thread      m_thread;
        std::mutex  m_mutex;
        std::condition_variable     m_cond;
        ThreadInitCallback          m_callback;

    public:

        EventLoopThread     (const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());
        ~EventLoopThread    ();
        EventLoop*  startLoop();

    private:
        void threadFunc();
};