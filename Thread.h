#pragma once

#include <functional>
#include <thread>
#include <unistd.h>
#include <atomic>
#include <memory>
#include <string>

#include "noncopyable.h"


class Thread: noncopyable
{

    using ThreadFunc = std::function<void()>;

    private:

        static  std::atomic_int32_t  sm_numCreated;

        bool        m_started   {false};
        bool        m_joined    {false};
        std::thread m_thread;
        pid_t       m_tid       {0};
        ThreadFunc  m_func;
        std::string m_name;

    public:

        static int32_t  numCreated  ();

        explicit    Thread  (ThreadFunc func, const std::string& name = std::string());
        ~Thread     ();

        void    start   ();
        void    join    ();

        bool    started () const;
        pid_t   tid     () const;
        const std::string&  name    () const;

    private:
        void setDefaultName ();
};