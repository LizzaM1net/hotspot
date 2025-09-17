@0xb8182c236c30d33d;

const hotspot :UInt32 = 0x1f6dc;
const wave :UInt32 = 0x1f44b;

struct Header {
    emoji @0 :UInt32;
}

struct SocketAddr {
    ip @0 :UInt32;
    port @1 :UInt16;
}

struct RouterCreateWaitroomRequest {
    addresses @0 :List(SocketAddr);
}
