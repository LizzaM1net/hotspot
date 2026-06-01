#include "hproto.h"
#include "hbuffer.h"

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
        return sizeof(h_size_t) + f.name.size() + sizeof(h_size_t) + f.data.size();
    }
    static bool hproto_accepts_size(size_t s) {
        // TODO: Maybe move size validation to read
        return s >= 2*sizeof(h_size_t);
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
