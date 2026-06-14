#pragma once

#include "hproto.h"

#include <netinet/in.h>

class HSocketAddress {
public:
    HSocketAddress(uint32_t ip, uint16_t port) {
        m_ip = htonl(ip);
        m_port = htons(port);
    }

    HSocketAddress(sockaddr_in address) {
        m_ip = address.sin_addr.s_addr;
        m_port = address.sin_port;
    }

    uint32_t ip() const { return ntohl(m_ip); }
    void setIp(uint32_t ip) { m_ip = htonl(ip); }

    uint16_t port() const { return ntohs(m_port); }
    void setPort(uint16_t port) { m_port = htons(port); }

    sockaddr_in sockaddr() const {
        return sockaddr_in {
            .sin_family = AF_INET,
            .sin_port   = m_port,
            .sin_addr   = { m_ip },
        };
    }

    auto operator<=>(const HSocketAddress& other) const {
        if (auto cmp = ntohl(m_ip) <=> ntohl(other.m_ip); cmp != 0)
            return cmp;
        return ntohs(m_port) <=> ntohs(other.m_port);
    }
    bool operator==(const HSocketAddress& other) const = default;

private:
    uint32_t m_ip;
    uint16_t m_port;
};
