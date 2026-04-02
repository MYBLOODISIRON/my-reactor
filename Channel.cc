#include <sys/epoll.h>

#include "Channel.h"
#include "Logger.h"


Channel::Channel(EventLoop* loop, int fd)
:m_loop {loop}, m_fd {fd}, m_events {0}, m_revents {0}, m_index {-1}, m_tied {false}
{

}

void Channel::tie(const std::shared_ptr<void>& obj)
{
    m_tie = obj;
    m_tied = true;
}

void Channel::update()
{

}

void Channel::remove()
{
}


void Channel::handleEvent(Timestamp reciveTime)
{
    if(m_tied)
    {
        std::shared_ptr<void> guard = m_tie.lock();
        if(guard)
        {
            handleEventWithGuard(reciveTime);
        }
    }
    else
    {
        handleEventWithGuard(reciveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp reciveTime)
{
    LOG_INFO("channel handleEvent revent: %d\n", m_revents)
    if((m_revents & EPOLLHUP) && !(EPOLLIN & m_revents))
    {
        if(m_closeCallback)
        {
            m_closeCallback();
        }
    }

    if(m_revents & EPOLLERR)
    {
        if(m_errorCallback)
        {
            m_errorCallback();
        }
    }

    if(m_revents & (EPOLLIN | EPOLLPRI))
    {
        if(m_readCallback)
        {
            m_readCallback();
        }
    }

    if(m_revents & EPOLLOUT)
    {
        if(m_writeCallback)
        {
            m_writeCallback();
        }
    }
}


void Channel::setReadCallback(ReadEventCallback cb)
{
    m_readCallback = std::move(cb);
}

void Channel::setWriteCallback(EventCallback cb)
{
    m_writeCallback = std::move(cb);
}

void Channel::setCloseCallback(EventCallback cb)
{
    m_closeCallback = std::move(cb);
}

void Channel::setErrorCallback(EventCallback cb)
{
    m_errorCallback = std::move(cb);
}

int Channel::fd() const
{
    return m_fd;
}

int Channel::events() const
{
    return m_events;
}

int Channel::set_revents(int revent)
{
    m_revents = revent;
}

void Channel::enableReading()
{
    m_events |= sm_kReadEvent;
    update();
}

void Channel::disableReading()
{
    m_events &= (~sm_kReadEvent);
    update();
}

void Channel::enableWriting()
{
    m_events |= sm_kWriteEvent;
    update();
}

void Channel::disableWriting()
{
    m_events &= (~sm_kWriteEvent);
    update();
}

void Channel::disableAll()
{
    m_events = sm_kNoneEvent;
    update();
}

bool Channel::isNoneEvent() const
{
    return m_events == sm_kNoneEvent;
}

bool Channel::isWriting() const
{
    return m_events & sm_kWriteEvent;
}

bool Channel::isReading() const
{
    return m_events & sm_kReadEvent;
}

int Channel::index()
{
    return m_index;
}

void Channel::setIndex(int index)
{
    m_index = index;
}

EventLoop* Channel::ownerLoop()
{
    return m_loop;
}