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

template <>
constexpr std::string_view hproto_name<HotspotFile>() {
    return "HotspotFile";
}

template <>
struct hproto_traits<HotspotFile> {
    static constexpr size_t size() {
        return hproto_traits<std::string>::size()
            + hproto_traits<std::vector<char>>::size();
    }

    static void write_impl(const HotspotFile &f, char **data) {
        hproto_traits<std::string>::write_impl(f.name, data);
        hproto_traits<std::vector<char>>::write_impl(f.data, data);
    }

    static HotspotFile read_impl(const char **data) {
        HotspotFile file;
        file.name = hproto_traits<std::string>::read_impl(data);
        file.data = hproto_traits<std::vector<char>>::read_impl(data);
        return file;
    }
};
