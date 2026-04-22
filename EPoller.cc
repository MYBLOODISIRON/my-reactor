#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>

#include "EPoller.h"
#include "Logger.h"
#include "Channel.h"


const int kNew {-1};    // channel的index成员变量，代表channel的状态
const int kAdded    {1};
const int kDeleted  {2};

EPoller::EPoller(EventLoop *loop)
:   Poller(loop), 
    m_epollfd   {epoll_create1(EPOLL_CLOEXEC)} 
{
    if(m_epollfd < 0)
    {
        LOG_FATAL("epoll_create fatal: %d\n", errno);
    }
}

EPoller::~EPoller()
{
    close(m_epollfd);
}

Timestamp EPoller::poll(int timeoutms, ChannelList *activeChannels)
{
    int numEvents {0};
    numEvents = epoll_wait(m_epollfd, m_events.data(), static_cast<int>(m_events.size()), timeoutms);
    int error_num {errno};
    Timestamp time_now {Timestamp::now()};
    if(numEvents > 0)
    {
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == m_events.size())
        {
            m_events.resize(m_events.size() * 2);
        }
    }
    else if(numEvents == 0)
    {

    }
    else
    {
        if(error_num != EINTR)
        {
            errno = error_num;
            LOG_ERROR("EPollPoller::poll() error.\n");
        }
    }
    return time_now;
}

void EPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for(int i = 0; i < numEvents; i ++)
    {
        Channel *channel {static_cast<Channel*>(m_events[i].data.ptr)};
        channel->set_revents(m_events[i].events);
        activeChannels->push_back(channel);
    }
}

void EPoller::updateChannel(Channel *channel)
{
    const int index {channel->index()};
    LOG_INFO("fd=%d events=%d index=%d \n",channel->fd(), channel->events(), index);
    if(index == kNew || index == kDeleted)
    {
        if(index == kNew)
        {
            int fd {channel->fd()};
            m_channels[fd] = channel;
        }

        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd {channel->fd()};
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }

}

void EPoller::removeChannel(Channel *channel)
{
    int fd {channel->fd()};
    int index {channel->index()};
    m_channels.erase(fd);
    LOG_INFO("fd=%d events=%d index=%d \n",channel->fd(), channel->events(), index);
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}


void EPoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd {channel->fd()};
    
    if(epoll_ctl(m_epollfd, operation, fd, &event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod fatal:%d\n", errno);
        }
    }
}