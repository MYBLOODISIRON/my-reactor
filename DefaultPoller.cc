#include <stdlib.h>

#include "Poller.h"
#include "EPoller.h"

Poller* Poller::newDefaultPoller(EventLoop *loop)   // 可能需要包含子类的头文件，所以不放在Poller.cc中
{
    if(getenv("MYREACTOR_USE_POLL"))
    {
        return nullptr;
    }
    else
    {
        return new EPoller {loop};
    }
}