#include "hudpchannel.h"

#include "hlog.h"

#include <netinet/in.h>
#include <unistd.h>

HUdpChannel::HUdpChannel(in_addr_t addr, uint16_t port) {
    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sockfd < 0) {
        hLog() << "Cannot create socket";
        return;
    }

    struct sockaddr_in sockAddr = {
        .sin_family = AF_INET,
        .sin_port   = htons(port),
        .sin_addr = { .s_addr = addr }
    };
    if (bind(m_sockfd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) < 0) {
        hLog() << "Cannot bind socket to port";
        close(m_sockfd);
        return;
    }
    m_valid = true;
}

ssize_t HUdpChannel::read(char *data, size_t size, sockaddr_in *inAddr) {
    socklen_t sockaddr_size = sizeof(sockaddr_in);
    return recvfrom(m_sockfd, data, size, 0,
                    (struct sockaddr *) inAddr, &sockaddr_size);
}

ssize_t HUdpChannel::write(char *data, size_t size, const sockaddr_in &toAddr) {
    return sendto(m_sockfd, data, size, 0,
                  reinterpret_cast<const sockaddr *>(&toAddr),
                  sizeof(toAddr));
}

bool HUdpChannel::isValid() {
    return m_valid;
}
