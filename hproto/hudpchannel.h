#pragma once

#include <netinet/in.h>
#include <cstdint>

class HUdpChannel {
public:
    HUdpChannel(in_addr_t addr, uint16_t port);

    ssize_t read(char *data, size_t size, sockaddr_in *inAddr);
    ssize_t write(char *data, size_t size, const sockaddr_in &inAddr);

    bool isValid();

private:
    bool m_valid = false;
    int m_sockfd = 0;
};
