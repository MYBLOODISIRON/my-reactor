#pragma once
#include <vector>
#include <sys/epoll.h>
#include "Poller.h"
class Channel;
class Timestamp;


class EPoller: public Poller
{

    using EventList = std::vector< struct epoll_event >;
    private:

        static const int    sm_kInitEventListSize {16};
        int                 m_epollfd;
        EventList           m_events    {sm_kInitEventListSize};

    public:
        EPoller     (EventLoop *loop);
        ~EPoller    () override;

        Timestamp   poll    (int timeoutms, ChannelList *activeChannels) override;
        void        updateChannel   (Channel *channel) override;
        void        removeChannel   (Channel *channel) override;

    private:
        void    fillActiveChannels  (int numEvents, ChannelList *activeChannels) const;
        void    update              (int operation, Channel *channel);
};