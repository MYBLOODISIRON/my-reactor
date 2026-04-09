#pragma once

#include <iostream>
#include <string>

class Timestamp
{
    private:

        int64_t m_second_since_epoch    {0};

    public:

        static  Timestamp   now     ();

        Timestamp   () = default;
        ~Timestamp  () = default;
        explicit    Timestamp   (int64_t second_since_epoch);
        std::string toString() const;
};