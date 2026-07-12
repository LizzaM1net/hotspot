#include <catch2/catch_test_macros.hpp>

#include <cassert>
#include <cstdint>
#include <hproto.h>
#include <hlog.h>

struct SimpleStruct {
    int32_t a, b, c;
};

struct PaddedStruct {
    int32_t a;
    double b;
    int32_t c;
};

struct EmptyStruct {};

TEST_CASE("hproto_write serializes SimpleStruct correctly", "[hproto]") {
    hLog() << "Testing SimpleStruct";
    SimpleStruct simpleObj{1, 2, 3};
    const char simpleExpected[] = {
        0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
    };
    char simpleReal[sizeof(SimpleStruct)];
    hproto_write(simpleObj, simpleReal);
    CHECK(memcmp(simpleReal, simpleExpected, sizeof(SimpleStruct)) == 0);
}

TEST_CASE("hproto_write serializes PaddedStruct correctly, zeroing padding", "[hproto]") {
    hLog() << "Testing PaddedStruct";
    PaddedStruct paddedObj;
    // Immitates uninitialized data before constructor run
    std::memset(&paddedObj, 0xfe, sizeof(PaddedStruct));
    std::construct_at(&paddedObj, 1, 2.0, 3);
    const char paddedExpected[] = {
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    char paddedReal[sizeof(PaddedStruct)];
    hproto_write(paddedObj, paddedReal);
    CHECK(memcmp(paddedReal, paddedExpected, sizeof(PaddedStruct)) == 0);
}

TEST_CASE("hproto_write serializes EmptyStruct correctly", "[hproto]") {
    hLog() << "Testing EmptyStruct";
    EmptyStruct emptyObj;
    const char emptyExpected[] = {
        0x00
    };
    char emptyReal[sizeof(EmptyStruct)];
    hproto_write(emptyObj, emptyReal);
    CHECK(memcmp(emptyReal, emptyExpected, sizeof(EmptyStruct)) == 0);
}
