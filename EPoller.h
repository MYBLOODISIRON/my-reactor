#pragma once

#include <vector>
#include <sys/epoll.h>

#include "Poller.h"

class Channel;

class EPoller: public Poller
{
    private:
        using EventList = std::vector<epoll_event>;

        static const int    kInitEventListSize {16};
        int         m_epollfd;
        EventList   m_events;

    public:
        EPoller     (EventLoop *loop);
        ~EPoller    () override;

        Timestamp   poll    (int timeoutms, ChannelList *activeChannels) override;
        void        updateChannel   (Channel *channel) override;
        void        removeChannel   (Channel *channel) override;

    private:
        void    fillActiveChannels  (int numEvents, ChannelList *activeChannels) const;
        void    update  (int operation, Channel *channel);
};