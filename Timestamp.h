#pragma once

#include <iostream>
#include <string>

class Timestamp
{
    private:
        int64_t     m_micro_second_since_epoch;
    public:
        Timestamp() = default;
        explicit Timestamp(int64_t microSecondSinceEpoch);
        static Timestamp now();
        std::string toString() const;
};