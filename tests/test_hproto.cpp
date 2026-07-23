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

struct NestedStruct {
    SimpleStruct a;
    EmptyStruct b;
};

TEST_CASE("hproto_write serializes int correctly", "[hproto]") {
    hLog() << "Testing int";
    int number = 10;
    const unsigned char numberExpected[] = {
        0x0A, 0x00, 0x00, 0x00,
    };
    char numberReal[hproto_size<int>()];
    char *ptr = numberReal;
    hproto_write(number, &ptr);
    REQUIRE(hproto_size<int>() == 4);
    REQUIRE(memcmp(numberReal, numberExpected, hproto_size<int>()) == 0);
}

TEST_CASE("hproto_write serializes double correctly", "[hproto]") {
    hLog() << "Testing double";
    double number = 1.42;
    const unsigned char numberExpected[] = {
        0xB8, 0x1E, 0x85, 0xEB,
        0x51, 0xB8, 0xF6, 0x3F,
    };
    char numberReal[hproto_size<double>()];
    char *ptr = numberReal;
    hproto_write(number, &ptr);
    REQUIRE(hproto_size<double>() == 8);
    REQUIRE(memcmp(numberReal, numberExpected, hproto_size<double>()) == 0);
}

TEST_CASE("hproto_write serializes SimpleStruct correctly", "[hproto]") {
    hLog() << "Testing SimpleStruct";
    SimpleStruct simpleObj{1, 2, 3};
    const unsigned char simpleExpected[] = {
        0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
    };
    char simpleReal[hproto_size<SimpleStruct>()];
    char *ptr = simpleReal;
    hproto_write(simpleObj, &ptr);
    REQUIRE(hproto_size<SimpleStruct>() == 12);
    REQUIRE(memcmp(simpleReal, simpleExpected, hproto_size<SimpleStruct>()) == 0);
}

TEST_CASE("hproto_write serializes PaddedStruct correctly, zeroing padding", "[hproto]") {
    hLog() << "Testing PaddedStruct";
    PaddedStruct paddedObj;
    // Immitates uninitialized data before constructor run
    std::memset(&paddedObj, 0xfe, sizeof(PaddedStruct));
    std::construct_at(&paddedObj, 1, 2.0, 3);
    const unsigned char paddedExpected[] = {
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    char paddedReal[hproto_size<PaddedStruct>()];
    char *ptr = paddedReal;
    hproto_write(paddedObj, &ptr);
    REQUIRE(hproto_size<PaddedStruct>() == 24);
    REQUIRE(memcmp(paddedReal, paddedExpected, hproto_size<PaddedStruct>()) == 0);
}

TEST_CASE("hproto_write serializes EmptyStruct correctly", "[hproto]") {
    hLog() << "Testing EmptyStruct";
    EmptyStruct emptyObj;
    const unsigned char emptyExpected[] = {
        0x00
    };
    char emptyReal[sizeof(EmptyStruct)];
    char *ptr = emptyReal;
    hproto_write(emptyObj, &ptr);
    REQUIRE(hproto_size<EmptyStruct>() == 1);
    REQUIRE(memcmp(emptyReal, emptyExpected, sizeof(EmptyStruct)) == 0);
}

TEST_CASE("hproto_write serializes NestedStruct correctly", "[hproto]") {
    hLog() << "Testing NestedStruct";
    NestedStruct nestedObj{
        {3, 2, 1}
    };
    const unsigned char nestedExpected[] = {
        0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    char nestedReal[sizeof(NestedStruct)];
    char *ptr = nestedReal;
    hproto_write(nestedObj, &ptr);
    REQUIRE(hproto_size<NestedStruct>() == 16);
    REQUIRE(memcmp(nestedReal, nestedExpected, sizeof(NestedStruct)) == 0);
}
