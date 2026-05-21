#include <catch2/catch_all.hpp>

#include "hproto.h"
#include "hproto_types.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <variant>
#include <vector>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

template <typename... Ts>
static std::vector<uint8_t> serialize(std::variant<Ts...> v) {
    std::vector<uint8_t> buf(hproto_size(v));
    hproto_write(v, buf.data());
    return buf;
}

// Verify the 4-byte little-endian type ID at the start of a serialized buffer.
static uint32_t read_type_id(const std::vector<uint8_t> &buf) {
    uint32_t id;
    std::memcpy(&id, buf.data(), sizeof(id));
    return id;
}

// ---------------------------------------------------------------------------
// std::string
// ---------------------------------------------------------------------------

TEST_CASE("string: round-trip", "[hproto][string]") {
    SECTION("normal ASCII") {
        std::variant<std::string> v = std::string("hello world");
        auto buf = serialize(v);
        auto result = hproto_read<std::string>(buf.data(), buf.size());
        REQUIRE(std::get<std::string>(result) == "hello world");
    }

    SECTION("empty string") {
        std::variant<std::string> v = std::string("");
        auto buf = serialize(v);
        auto result = hproto_read<std::string>(buf.data(), buf.size());
        REQUIRE(std::get<std::string>(result) == "");
    }

    SECTION("unicode / multibyte") {
        // Plain literal: source file is UTF-8, so this is the same bytes.
        std::string utf8 = "\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82"
                           " \xD0\xBC\xD0\xB8\xD1\x80";  // "Привет мир"
        std::variant<std::string> v = utf8;
        auto buf = serialize(v);
        auto result = hproto_read<std::string>(buf.data(), buf.size());
        REQUIRE(std::get<std::string>(result) == utf8);
    }

    SECTION("string containing null bytes") {
        std::string s("abc\0def", 7);
        std::variant<std::string> v = s;
        auto buf = serialize(v);
        auto result = hproto_read<std::string>(buf.data(), buf.size());
        REQUIRE(std::get<std::string>(result) == s);
    }
}

TEST_CASE("string: wire format", "[hproto][string]") {
    std::string text = "hi";
    std::variant<std::string> v = text;
    auto buf = serialize(v);

    // [u32 LE type_id=0x0002][u32 LE len=2][h][i]
    REQUIRE(buf.size() == 4 + 4 + 2);
    REQUIRE(read_type_id(buf) == 0x0002u);

    uint32_t len;
    std::memcpy(&len, buf.data() + 4, 4);
    REQUIRE(len == 2u);
    REQUIRE(buf[8] == 'h');
    REQUIRE(buf[9] == 'i');
}

// ---------------------------------------------------------------------------
// RouterCreateWaitroomRequest
// ---------------------------------------------------------------------------

TEST_CASE("RouterCreateWaitroomRequest: round-trip", "[hproto][router]") {
    RouterCreateWaitroomRequest req{0x7F000001u, 12345u, 0u};
    std::variant<RouterCreateWaitroomRequest> v = req;
    auto buf = serialize(v);

    auto result = hproto_read<RouterCreateWaitroomRequest>(buf.data(), buf.size());
    auto *r = std::get_if<RouterCreateWaitroomRequest>(&result);
    REQUIRE(r != nullptr);
    REQUIRE(r->local_ip   == 0x7F000001u);
    REQUIRE(r->local_port == 12345u);
}

TEST_CASE("RouterCreateWaitroomRequest: wire format", "[hproto][router]") {
    RouterCreateWaitroomRequest req{0x01020304u, 9999u, 0u};
    std::variant<RouterCreateWaitroomRequest> v = req;
    auto buf = serialize(v);

    // [u32 LE type_id=0x1002][u32 LE ip][u16 LE port][u16 pad]
    REQUIRE(buf.size() == 4 + 8);
    REQUIRE(read_type_id(buf) == 0x1002u);
}

// ---------------------------------------------------------------------------
// RouterRedirectAnswer
// ---------------------------------------------------------------------------

TEST_CASE("RouterRedirectAnswer: round-trip", "[hproto][router]") {
    RouterRedirectAnswer ans{0xC0A80101u, 54321u, 0u};  // 192.168.1.1:54321
    std::variant<RouterRedirectAnswer> v = ans;
    auto buf = serialize(v);

    auto result = hproto_read<RouterRedirectAnswer>(buf.data(), buf.size());
    auto *r = std::get_if<RouterRedirectAnswer>(&result);
    REQUIRE(r != nullptr);
    REQUIRE(r->ip   == 0xC0A80101u);
    REQUIRE(r->port == 54321u);
}

TEST_CASE("RouterRedirectAnswer: type ID", "[hproto][router]") {
    RouterRedirectAnswer ans{0u, 0u, 0u};
    std::variant<RouterRedirectAnswer> v = ans;
    auto buf = serialize(v);
    REQUIRE(read_type_id(buf) == 0x1003u);
}

// ---------------------------------------------------------------------------
// RouterGreet
// ---------------------------------------------------------------------------

TEST_CASE("RouterGreet: serializes to type ID only", "[hproto][greet]") {
    std::variant<RouterGreet> v = RouterGreet{};
    auto buf = serialize(v);

    REQUIRE(buf.size() == 4u);  // only the 4-byte type ID
    REQUIRE(read_type_id(buf) == 0x1004u);
}

TEST_CASE("RouterGreet: round-trip", "[hproto][greet]") {
    std::variant<RouterGreet> v = RouterGreet{};
    auto buf = serialize(v);

    auto result = hproto_read<RouterGreet>(buf.data(), buf.size());
    REQUIRE(std::get_if<RouterGreet>(&result) != nullptr);
}

// ---------------------------------------------------------------------------
// Unknown type ID → monostate
// ---------------------------------------------------------------------------

TEST_CASE("unknown type ID yields monostate", "[hproto][unknown]") {
    // Craft a datagram with an unknown type ID.
    std::vector<uint8_t> buf(4, 0);
    uint32_t bad_id = 0xDEADBEEFu;
    std::memcpy(buf.data(), &bad_id, 4);

    auto result = hproto_read<std::string, RouterCreateWaitroomRequest,
                              RouterRedirectAnswer, RouterGreet>(buf.data(), buf.size());
    REQUIRE(std::holds_alternative<std::monostate>(result));
}

// ---------------------------------------------------------------------------
// Multi-type dispatch: correct type matched
// ---------------------------------------------------------------------------

TEST_CASE("multi-type variant dispatches to the right type", "[hproto][dispatch]") {
    RouterRedirectAnswer ans{0x01010101u, 8080u, 0u};
    std::variant<RouterRedirectAnswer> vsend = ans;
    auto buf = serialize(vsend);

    // Try to read as all types; only RouterRedirectAnswer should match.
    auto result = hproto_read<std::string, RouterCreateWaitroomRequest,
                              RouterRedirectAnswer, RouterGreet>(buf.data(), buf.size());

    REQUIRE(!std::holds_alternative<std::monostate>(result));
    REQUIRE(std::holds_alternative<RouterRedirectAnswer>(result));
    auto &r = std::get<RouterRedirectAnswer>(result);
    REQUIRE(r.ip   == 0x01010101u);
    REQUIRE(r.port == 8080u);
}

TEST_CASE("multi-type variant: string does not match router types", "[hproto][dispatch]") {
    std::variant<std::string> vsend = std::string("test");
    auto buf = serialize(vsend);

    // Try to deserialize as router types only (no std::string in the list).
    auto result = hproto_read<RouterCreateWaitroomRequest,
                              RouterRedirectAnswer, RouterGreet>(buf.data(), buf.size());
    REQUIRE(std::holds_alternative<std::monostate>(result));
}
