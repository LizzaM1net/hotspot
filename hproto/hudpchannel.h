#pragma once

#include <netinet/in.h>
#include <cstdint>
#include <functional>
#include <map>
#include <coroutine>

#include "hproto_types.h"

struct ReadAwaitable;

struct Task {
    struct promise_type {
        ReadAwaitable *pending = nullptr;

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;
};

struct ReadAwaitable {
    char *data = nullptr;
    size_t size = 0;
    ssize_t readSize = -1;

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<Task::promise_type> handle) {
        handle.promise().pending = this;
    }
    ssize_t await_resume() { return readSize; }
};

class HUdpServer;

class HUdpChannel {
    friend class HUdpServer;
public:
    HUdpChannel(const HUdpChannel&) = delete;
    HUdpChannel& operator=(const HUdpChannel&) = delete;

    void finishLater();

    ReadAwaitable read(char *data, size_t size);
    ssize_t write(const char *data, size_t size);

    HSocketAddress peer() const;

private:
    HUdpChannel(HUdpServer *server, HSocketAddress peer,
                std::function<Task(HUdpChannel *)> handler);
    ~HUdpChannel();

    std::coroutine_handle<Task::promise_type> m_handle;

    HSocketAddress m_peer;
    HUdpServer *m_server;
};

class HUdpServer {
    friend class HUdpChannel;
public:
    HUdpServer(in_addr_t addr, uint16_t port,
               std::function<Task(HUdpChannel *)> newHandler);
    ~HUdpServer();

    bool isValid();
    HSocketAddress address();
    bool processDatagram();
    int sockfd();

    HUdpChannel *channelToAddress(HSocketAddress address);
    void finishLater(HUdpChannel *channel);

private:
    ssize_t write(const char *data, size_t size, HSocketAddress addr);

    bool m_valid = false;
    int m_sockfd = 0;

    std::function<Task(HUdpChannel *)> m_handler;
    std::map<HSocketAddress, HUdpChannel *> m_chans;
    std::vector<HUdpChannel *> m_finished;
};
