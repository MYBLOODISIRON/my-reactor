#include "Timestamp.h"
#include <time.h>

Timestamp::Timestamp()
:m_micro_second_since_epoch{0}
{

}

Timestamp::Timestamp(int64_t microSecondSinceEpoch)
:m_micro_second_since_epoch{microSecondSinceEpoch}
{

}

Timestamp Timestamp::now()
{
    time_t tm {time(nullptr)};
    return Timestamp(tm);
}


std::string Timestamp::toString() const
{
    char buffer[128];
    tm* tm_time {localtime(&m_micro_second_since_epoch)};
    snprintf(buffer, 128, "%4d/%02d/%02d %02d:%02d:%02d ", tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);

    return buffer;
}