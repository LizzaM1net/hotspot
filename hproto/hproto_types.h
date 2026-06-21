#pragma once

#include "hproto.h"

#include <netinet/in.h>

class HSocketAddress {
public:
    HSocketAddress(uint32_t ip, uint16_t port) {
        m_ip = ip;
        m_port = port;
    }

    HSocketAddress(sockaddr_in address) {
        m_ip = ntohl(address.sin_addr.s_addr);
        m_port = ntohs(address.sin_port);
    }

    uint32_t ip() const { return m_ip; }
    void setIp(uint32_t ip) { m_ip = ip; }

    uint16_t port() const { return m_port; }
    void setPort(uint16_t port) { m_port = port; }

    sockaddr_in sockaddr() const {
        return sockaddr_in {
            .sin_family = AF_INET,
            .sin_port   = htons(m_port),
            .sin_addr   = { htonl(m_ip) },
        };
    }

    auto operator<=>(const HSocketAddress& other) const {
        if (auto cmp = m_ip <=> other.m_ip; cmp != 0)
            return cmp;
        return m_port <=> other.m_port;
    }
    bool operator==(const HSocketAddress& other) const = default;

private:
    uint32_t m_ip;
    uint16_t m_port;
};
