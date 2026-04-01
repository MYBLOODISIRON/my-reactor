#pragma once

#include <functional>
#include <sys/epoll.h>
#include <memory>
#include "noncopyable.h"
#include "Timestamp.h"

class EventLoop; // 前置声明


class Channel: noncopyable
{
    private:
        static const int sm_kNoneEvent  {0};
        static const int sm_kReadEvent  {EPOLLIN | EPOLLPRI};
        static const int sm_kWriteEvent {EPOLLOUT};

        EventLoop   *m_loop; // 事件循环
        const int   m_fd;   // 监听对象
        int         m_events;
        int         m_revents;
        int         m_index;

        std::weak_ptr<void>     m_tie;
        bool                    m_tied;

        ReadEventCallback       m_readCallback;
        EventCallback           m_writeCallback;
        EventCallback           m_closeCallback;
        EventCallback           m_errorCallback;
    public:
        using EventCallback     = std::function<void()>;
        using ReadEventCallback = std::function<void(Timestamp)>;

        Channel     (EventLoop* loop, int fd);
        ~Channel    () = default;

        void handleEvent        (Timestamp reciveTime);
        void setReadCallback    (ReadEventCallback cb);
        void setWriteCallback   (EventCallback cb);
        void setCloseCallback   (EventCallback cb);
        void setErrorCallback   (EventCallback cb);

        void    tie (const std::shared_ptr<void>&);
        int     fd  () const;
        int     events  () const;
        int     set_revents (int revent);
        void    enableReading   (); // 设置fd的事件
        void    disableReading  ();
        void    enableWriting   ();
        void    disableWriting  ();
        void    disableAll  ();

        bool    isNoneEvent () const;
        bool    isWriting   () const;
        bool    isReading   () const;
        int     index   ();
        void    setIndex    (int index);
        EventLoop*  ownerLoop   ();
        void    remove  (); // 所属的EventLoop中删除该Channel
    private:
        void    update  (); // 当改变Channel所表示fd的Events事件后，update负责在poller里面更改相应fd的事件
        void    handleEventWithGuard    ();
};