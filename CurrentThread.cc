#include "CurrentThread.h"

namespace CurrentThread
{
    __thread pthread_t cachedTid {0};


    void cacheTid()
    {
        cachedTid = pthread_self();
    }
}