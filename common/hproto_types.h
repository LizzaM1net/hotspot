#ifndef HPROTO_TYPES_H
#define HPROTO_TYPES_H

#include "hproto.h"
#include <cstdint>

// Sent by client to router: "I'm here, waiting for a peer"
struct RouterCreateWaitroomRequest {
    uint32_t local_ip;
    uint16_t local_port;
    uint16_t _pad;
};
HOTSPOT_SIZED_OBJECT(RouterCreateWaitroomRequest, 0x1002, 8)

// Sent by router to client: "your peer is at this address"
struct RouterRedirectAnswer {
    uint32_t ip;
    uint16_t port;
    uint16_t _pad;
};
HOTSPOT_SIZED_OBJECT(RouterRedirectAnswer, 0x1003, 8)

// Sent peer-to-peer after redirect: initial handshake ping
struct RouterGreet {};
HOTSPOT_EMPTY_OBJECT(RouterGreet, 0x1004)

#endif // HPROTO_TYPES_H
