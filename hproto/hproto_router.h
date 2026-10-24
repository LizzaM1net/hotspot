#pragma once

#include "hproto.h"
#include "hbuffer.h"
#include "hproto_types.h"

#include <vector>

struct RouterCreateWaitroomRequest {
    HSocketAddress localAddress;
};

struct RouterRedirectAnswer {
    HSocketAddress peerAddress;
};

struct RouterGreet {};

struct HotspotFile {
    std::string name;
    std::vector<char> data;
};
