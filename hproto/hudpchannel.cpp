#include "hudpchannel.h"

#include "hlog.h"
#include "hproto_types.h"

#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

HUdpChannel::HUdpChannel(HUdpServer *server, HSocketAddress peer,
                         std::function<Task(HUdpChannel *)> handler)
    : m_peer(peer)
    , m_server(server) {
    m_handle = handler(this).handle;
}

HUdpChannel::~HUdpChannel() {
    m_handle.destroy();
}

void HUdpChannel::finishLater() {
    m_server->finishLater(this);
}

ReadAwaitable HUdpChannel::read(char *data, size_t size) {
    return {data, size};
}

ssize_t HUdpChannel::write(const char *data, size_t size) {
    return m_server->write(data, size, peer());
}

HSocketAddress HUdpChannel::peer() const {
    return m_peer;
}

HUdpServer::HUdpServer(in_addr_t addr, uint16_t port,
           std::function<Task(HUdpChannel *)> handler) {
    m_handler = handler;

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

HUdpServer::~HUdpServer() {
    for (auto& [sender, channel] : m_chans)
        delete channel;

    if (m_valid)
        close(m_sockfd);
}

bool HUdpServer::isValid() {
    return m_valid;
}

HSocketAddress HUdpServer::address() {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(m_sockfd, (struct sockaddr *)&sin, &len) == -1) {
        perror("getsockname");
        // TODO: workaround exception here
    }

    return {sin};
}

bool HUdpServer::processDatagram() {
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    char dummy[1];

    if (recvfrom(m_sockfd, dummy, sizeof(dummy),
                MSG_PEEK,
                (struct sockaddr *)&sender, &sender_len) < 0) {
        if (errno == EINTR || errno == EWOULDBLOCK)
            return true;
        hLog() << "Fatal socket error:" << strerror(errno);
        return false;
    }

    HUdpChannel *channel = m_chans[sender];
    if (channel == nullptr) {
        channel = new HUdpChannel(this, sender, m_handler);
        m_chans[sender] = channel;
    }
    ReadAwaitable *pending = channel->m_handle.promise().pending;
    if (pending)
        pending->readSize = recv(m_sockfd, pending->data,
                                pending->size, 0);
    else // Skip datagram if channel dont want to eat
        recv(m_sockfd, dummy, sizeof(dummy), 0);
    channel->m_handle.resume();

    if (channel->m_handle.done()) {
        hLog() << "coroutine done";
        delete channel;
        m_chans.erase(sender);
    }

    for (const HUdpChannel *finished : m_finished) {
        for (auto& [sender, chan] : m_chans) {
            if (chan == finished) {
                delete chan;
                m_chans.erase(sender);
                break;
            }
        }
    }
    m_finished.clear();

    return true;
}

int HUdpServer::sockfd() {
    return m_sockfd;
}

HUdpChannel *HUdpServer::channelToAddress(HSocketAddress address) {
    HUdpChannel *channel = m_chans[address];
    if (channel == nullptr) {
        channel = new HUdpChannel(this, address, m_handler);
        m_chans[address] = channel;
    }
    return channel;
}

void HUdpServer::finishLater(HUdpChannel *channel) {
    m_finished.push_back(channel);
}

ssize_t HUdpServer::write(const char *data, size_t size, HSocketAddress addr) {
    sockaddr_in address = addr.sockaddr();
    return sendto(m_sockfd, data, size, 0,
                  reinterpret_cast<const sockaddr *>(&address),
                  sizeof(address));
}
