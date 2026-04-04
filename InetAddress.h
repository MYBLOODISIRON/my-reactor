#pragma once
#include <string>
#include <arpa/inet.h>


class InetAddress
{
    private:
        struct sockaddr_in m_addr;
    public:
        explicit InetAddress(uint16_t port, const std::string& ip);
        explicit InetAddress(const sockaddr_in& addr);
        std::string toIp() const;
        std::string toIpPort() const;
        uint16_t toPort() const;
        const sockaddr_in* getSockAddr() const;
        void    setSockAddr(const struct sockaddr_in& addr);
};