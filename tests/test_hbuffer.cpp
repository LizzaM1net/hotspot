#include <catch2/catch_test_macros.hpp>

#include <hbuffer.h>
#include <hlog.h>

TEST_CASE("hproto_write serializes std::string correctly", "[hproto]") {
    hLog() << "Testing std::string";
    std::string string = "abc 123";
    const unsigned char stringExpected[] = {
        0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x61, 0x62, 0x63, 0x20, 0x31, 0x32, 0x33, 0x00
    };
    char stringReal[hproto_size(string)];
    hproto_write<std::string>(string, stringReal);
    REQUIRE(hproto_size(string) == 16);
    REQUIRE(memcmp(stringReal, stringExpected, 16) == 0);
}
