#include "InetAddress.h"
#include <strings.h>
#include <string.h>

InetAddress::InetAddress(uint16_t port, const std::string& ip)
{
    bzero(&m_addr, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const
{
    char buffer[64] {0};
    inet_ntop(AF_INET, &m_addr.sin_addr, buffer, 64);
    return buffer;
}

std::string InetAddress::toIpPort() const
{
    char buffer[64] {0};
    inet_ntop(AF_INET, &m_addr.sin_addr, buffer, 64);
    size_t length {strlen(buffer)};
    uint16_t port {ntohs(m_addr.sin_port)};
    sprintf(buffer + length, ":%u", port);
    return buffer;
}


uint16_t InetAddress::toPort() const
{
    return ntohs(m_addr.sin_port);
}


void InetAddress::setSockAddr(const struct sockaddr_in& addr)
{
    m_addr = addr;
}


InetAddress::InetAddress(const sockaddr_in& addr)
{
    m_addr = addr;
}


const sockaddr_in* InetAddress::getSockAddr() const
{
    return &m_addr;
}