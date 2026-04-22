#pragma once
#include <vector>
#include <unordered_map>
#include "noncopyable.h"
class EventLoop;
class Timestamp;
class Channel;



class Poller: noncopyable
{
    protected:
        using ChannelMap = std::unordered_map < int, Channel* >;
        using ChannelList = std::vector < Channel* >;

    private:

        EventLoop*  m_ownerLoop;

    protected:
        ChannelMap  m_channels;

    public:

        Poller  (EventLoop *loop);
        virtual ~Poller () = default;

        static  Poller*     newDefaultPoller(EventLoop *loop);   // 可能需要包含子类的头文件，所以不放在Poller.cc中

        virtual Timestamp   poll    (int timeoutms, ChannelList* activeChannels) = 0;
        virtual void        updateChannel   (Channel *channel) = 0;
        virtual void        removeChannel   (Channel *channel) = 0;

        bool    hasChannel  (Channel *channel) const;
};