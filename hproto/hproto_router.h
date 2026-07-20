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

template<>
struct HProtoData<HotspotFile> {
    static constexpr size_t hproto_size(const HotspotFile &f) {
        return sizeof(h_size_t) + f.name.size() + sizeof(h_size_t) + f.data.size();
    }
    static void hproto_write(const HotspotFile &f, char *data) {
        write_blob(data, f.name);
        write_blob(data, f.data);
    }
    static HotspotFile hproto_read(const char* data) {
        HotspotFile file;
        file.name = read_blob<std::string>(data);
        file.data = read_blob<std::vector<char>>(data);
        return file;
    }
};
