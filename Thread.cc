#include <semaphore.h>

#include "Thread.h"
#include "CurrentThread.h"


std::atomic_int32_t Thread::sm_numCreated {0};


Thread::Thread(ThreadFunc func, const std::string& name)
:   m_func {std::move(func)},
    m_name {name}
{
    setDefaultName();
}


Thread::~Thread()
{
    if(m_started && !m_joined)
    {
        m_thread.detach();
    }
}


void Thread::start()
{
    m_started = true;

    sem_t sem;
    sem_init(&sem, false, 0);

    m_thread = std::thread( // 开启线程
        [&] () {
            m_tid = CurrentThread::tid();
            sem_post(&sem);
            m_func();
        }
    );

    sem_wait(&sem); // 确保调用start方法后能放行使用m_tid
}


void Thread::join()
{
    m_joined = true;
    if(m_thread.joinable())
    {
        m_thread.join();
    }
}


int32_t Thread::numCreated()
{
    return sm_numCreated;
}


bool Thread::started() const
{
    return m_started;
}

pid_t Thread::tid() const
{
    return m_tid;
}

const std::string& Thread::name() const
{
    return m_name;
}

void Thread::setDefaultName()
{
    int num {++sm_numCreated};
    if(m_name.empty())
    {
        char buffer[32];
        snprintf(buffer, 32, "Thread%d", num);
        m_name = buffer;
    }
}