#include <memory>
#include "EventLoopThread.h"
#include "EventLoop.h"


EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
:   m_thread    { std::bind(&EventLoopThread::threadFunc, this), name },
    m_callback  {cb}
{

}


EventLoopThread::~EventLoopThread()
{
    m_exiting = true;
    if(m_loop != nullptr)
    {
        m_loop->quit();
        m_thread.join();
    }
}


EventLoop* EventLoopThread::startLoop()
{
    m_thread.start();
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex>    lock    {m_mutex};
        while(m_loop == nullptr)
        {
            m_cond.wait(lock);  // 等待新线程设置m_loop
        }
        loop = m_loop;
    }
    return loop;
}


void EventLoopThread::threadFunc()  // 线程入口函数
{
    EventLoop loop; // one loop per thread
    if(m_callback)
    {
        m_callback(&loop);
    }

    {
        std::unique_lock<std::mutex>    lock    {m_mutex};
        m_loop = &loop;
        m_cond.notify_one();
    }
    loop.loop();
    
    {   // loop exit
        std::unique_lock<std::mutex>    lock    {m_mutex};
        m_loop = nullptr;
    }
}