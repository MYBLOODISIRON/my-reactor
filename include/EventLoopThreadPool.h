#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "noncopyable.h"
#include "EventLoopThread.h"
class EventLoop;

class EventLoopThreadPool: noncopyable
{
    using ThreadInitCallback = std::function< void(EventLoop*) >;

    private:

        EventLoop   *m_baseLoop {nullptr};
        std::string m_name;
        bool        m_started   {false};
        int         m_numThreads    {0};
        int         m_next          {0};
        std::vector< std::unique_ptr< EventLoopThread > >   m_threads;
        std::vector< EventLoop* >                           m_loops;

    public:

        EventLoopThreadPool (EventLoop *baseLoop, const std::string &nameArg);
        ~EventLoopThreadPool    () = default;   // EventLoop全都是线程栈上的变量

        void    setNumThreads   (int numThreads);
        void    start(const ThreadInitCallback &cb = ThreadInitCallback());
        EventLoop*  getNextLoop (); // 多线程中，baseLoop默认以轮询分配channel给subloop
        std::vector< EventLoop* >   getAllLoops ()  const;

        bool    started () const;
        const std::string&  name    () const;
};