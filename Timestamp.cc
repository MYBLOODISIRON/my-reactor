#include "Timestamp.h"
#include <time.h>

Timestamp Timestamp::now()
{
    return Timestamp { time(nullptr) };
}

Timestamp::Timestamp(int64_t second_since_epoch)
:   m_second_since_epoch    {second_since_epoch}
{

}


std::string Timestamp::toString() const
{
    return ctime(&m_second_since_epoch);
}