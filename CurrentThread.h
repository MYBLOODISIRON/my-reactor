#pragma once
#include <pthread.h>



namespace CurrentThread
{
    extern __thread pthread_t cachedTid;
    void    cacheTid    ();
    inline  pthread_t   tid ()
    {
        if(cachedTid == 0)
        {
            cacheTid();
        }
        return cachedTid;
    }   
}