#include "hproto.h"

#include <vector>

struct RouterCreateWaitroomRequest {
    uint32_t local_ip;
    uint16_t local_port;
};
HOTSPOT_SIZED_OBJECT(RouterCreateWaitroomRequest, 0x1002, 8)

struct RouterRedirectAnswer {
    uint32_t ip;
    uint16_t port;
};
HOTSPOT_SIZED_OBJECT(RouterRedirectAnswer, 0x1003, 8)

struct RouterGreet {};
HOTSPOT_EMPTY_OBJECT(RouterGreet, 0x1004)

struct HotspotFile {
    std::string name;
    std::vector<char> data;
};

template<>
struct HProtoData<HotspotFile> {
    static constexpr const hproto_id_t hproto_id = 0x2001;
    static constexpr size_t hproto_size(const HotspotFile &f) {
        return HProtoData<std::string>::hproto_size(f.name) + sizeof(string_len_t) + f.data.size();
    }
    static bool hproto_accepts_size(size_t s) {
        // TODO: Maybe move size validation to read
        return s >= 2*sizeof(string_len_t);
    }
    static void hproto_write(const HotspotFile &f, void *data) {
        size_t stringSize = HProtoData<std::string>::hproto_size(f.name);
        HProtoData<std::string>::hproto_write(f.name, data);
        string_len_t dataSize = f.data.size();
        memcpy((char*)data+stringSize, &dataSize, sizeof(string_len_t));
        memcpy((char*)data+stringSize+sizeof(string_len_t), f.data.data(), dataSize);
    }
    static HotspotFile hproto_read(const char* data) {
        HotspotFile file;
        file.name = HProtoData<std::string>::hproto_read(data);
        size_t stringSize = HProtoData<std::string>::hproto_size(file.name);
        string_len_t dataSize;
        data += stringSize;
        memcpy(&dataSize, (char*)data, sizeof(string_len_t));
        data += sizeof(string_len_t);
        file.data = std::vector<char>((char*)data, (char*)data+dataSize);
        return file;
    }
};
