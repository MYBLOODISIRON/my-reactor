#include <memory>
#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"


EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg)
:   m_baseLoop  {baseLoop},
    m_name      {nameArg}
{

}


void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    m_started = true;
    
    for(int i = 0; i < m_numThreads; i ++)
    {
        const size_t buf_len {m_name.size() + 32};
        char buffer[buf_len];
        snprintf(buffer, sizeof buffer, "%s%d", m_name.c_str(), i);
        EventLoopThread *t = new EventLoopThread {cb, buffer};
        m_threads.emplace_back(t);
        m_loops.push_back(t->startLoop());
    }

    if(m_numThreads == 0 && cb)
    {
        cb(m_baseLoop);
    }
}


EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = m_baseLoop;
    if(! m_loops.empty())
    {
        loop = m_loops[m_next];
        m_next ++;
        if(m_next >= m_loops.size())
        {
            m_next = 0;
        }
    }
    return loop;
}


std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() const
{
    if(m_loops.empty())
    {
        return std::vector<EventLoop*> {1, m_baseLoop};
    }
    else
    {
        return m_loops;
    }
}


void EventLoopThreadPool::setNumThreads(int numThreads)
{
    m_numThreads = numThreads;
}


bool EventLoopThreadPool::started() const
{
    return m_started;
}


const std::string& EventLoopThreadPool::name() const
{
    return m_name;
}