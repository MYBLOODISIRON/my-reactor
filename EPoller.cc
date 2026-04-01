#include <errno.h>

#include "EPoller.h"
#include "Logger.h"


const int kNew {-1};
const int kAdd {1};
const int kDeleted {2};

EPoller::EPoller(EventLoop *loop)
: Poller(loop), m_epollfd {epoll_create1(EPOLL_CLOEXEC)}, m_events {kInitEventListSize}
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