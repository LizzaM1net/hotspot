#include <catch2/catch_test_macros.hpp>

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
    hLog() << "Testing int serialize";
    int number = 10;
    const unsigned char numberExpected[] = {
        0x0A, 0x00, 0x00, 0x00,
    };
    char numberReal[hproto_size(number)];
    hproto_write(number, numberReal);
    REQUIRE(sizeof(numberReal) == sizeof(numberExpected));
    REQUIRE(memcmp(numberReal, numberExpected, sizeof(numberReal)) == 0);
}

TEST_CASE("hproto_read deserializes int correctly", "[hproto]") {
    hLog() << "Testing int deserialize";
    const unsigned char numberData[] = {
        0x0A, 0x00, 0x00, 0x00,
    };

    int number = hproto_read<int>((char *)numberData);
    REQUIRE(hproto_size(number) == sizeof(numberData));
    REQUIRE(number == 10);
}

TEST_CASE("hproto_write serializes double correctly", "[hproto]") {
    hLog() << "Testing double serialize";
    double number = 1.42;
    const unsigned char numberExpected[] = {
        0xB8, 0x1E, 0x85, 0xEB,
        0x51, 0xB8, 0xF6, 0x3F,
    };
    char numberReal[hproto_size(number)];
    hproto_write(number, numberReal);
    REQUIRE(sizeof(numberReal) == sizeof(numberExpected));
    REQUIRE(memcmp(numberReal, numberExpected, sizeof(numberReal)) == 0);
}

TEST_CASE("hproto_write deserializes double correctly", "[hproto]") {
    hLog() << "Testing double deserialize";
    const unsigned char numberData[] = {
        0xB8, 0x1E, 0x85, 0xEB,
        0x51, 0xB8, 0xF6, 0x3F,
    };

    double number = hproto_read<double>((char*)numberData);
    REQUIRE(hproto_size(number) == sizeof(numberData));
    REQUIRE(number == 1.42);
}

TEST_CASE("hproto_write serializes SimpleStruct correctly", "[hproto]") {
    hLog() << "Testing SimpleStruct serialize";
    SimpleStruct simpleObj{1, 2, 3};
    const unsigned char simpleExpected[] = {
        0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
    };
    char simpleReal[hproto_size(simpleObj)];
    hproto_write(simpleObj, simpleReal);
    REQUIRE(sizeof(simpleReal) == sizeof(simpleExpected));
    REQUIRE(memcmp(simpleReal, simpleExpected, sizeof(simpleReal)) == 0);
}

TEST_CASE("hproto_write deserializes SimpleStruct correctly", "[hproto]") {
    hLog() << "Testing SimpleStruct deserialize";
    const unsigned char simpleData[] = {
        0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
    };

    SimpleStruct simple = hproto_read<SimpleStruct>((char*)simpleData);
    REQUIRE(hproto_size(simple) == sizeof(simpleData));
    REQUIRE(simple.a == 1);
    REQUIRE(simple.b == 2);
    REQUIRE(simple.c == 3);
}

TEST_CASE("hproto_write serializes PaddedStruct correctly, zeroing padding", "[hproto]") {
    hLog() << "Testing PaddedStruct serialize";
    PaddedStruct paddedObj{1, 2.0, 3};
    const unsigned char paddedExpected[] = {
        0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
        0x03, 0x00, 0x00, 0x00,
    };
    char paddedReal[hproto_size(paddedObj)];
    hproto_write(paddedObj, paddedReal);
    REQUIRE(sizeof(paddedReal) == sizeof(paddedExpected));
    REQUIRE(memcmp(paddedReal, paddedExpected, sizeof(paddedReal)) == 0);
}

TEST_CASE("hproto_write deserializes PaddedStruct correctly, zeroing padding", "[hproto]") {
    hLog() << "Testing PaddedStruct deserialize";
    const unsigned char paddedData[] = {
        0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
        0x03, 0x00, 0x00, 0x00,
    };

    PaddedStruct padded = hproto_read<PaddedStruct>((char*)paddedData);
    REQUIRE(hproto_size(padded) == sizeof(paddedData));
    REQUIRE(padded.a == 1);
    REQUIRE(padded.b == 2.0);
    REQUIRE(padded.c == 3);
}

TEST_CASE("hproto_write serializes EmptyStruct correctly", "[hproto]") {
    hLog() << "Testing EmptyStruct serialize";
    EmptyStruct emptyObj;
    const unsigned char emptyExpected[] = {};
    char emptyReal[hproto_size(emptyObj)];
    hproto_write(emptyObj, emptyReal);
    REQUIRE(sizeof(emptyReal) == sizeof(emptyExpected));
    REQUIRE(memcmp(emptyReal, emptyExpected, sizeof(emptyReal)) == 0);
}

TEST_CASE("hproto_write deserializes EmptyStruct correctly", "[hproto]") {
    hLog() << "Testing EmptyStruct deserialize";
    const unsigned char emptyData[] = {};

    EmptyStruct empty = hproto_read<EmptyStruct>((char*)emptyData);
    REQUIRE(hproto_size(empty) == sizeof(emptyData));
}

TEST_CASE("hproto_write serializes NestedStruct correctly", "[hproto]") {
    hLog() << "Testing NestedStruct serialize";
    NestedStruct nestedObj{
        {3, 2, 1}
    };
    const unsigned char nestedExpected[] = {
        0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
    };
    char nestedReal[hproto_size(nestedObj)];
    hproto_write(nestedObj, nestedReal);
    REQUIRE(sizeof(nestedReal) == sizeof(nestedExpected));
    REQUIRE(memcmp(nestedReal, nestedExpected, sizeof(nestedReal)) == 0);
}

TEST_CASE("hproto_write deserializes NestedStruct correctly", "[hproto]") {
    hLog() << "Testing NestedStruct deserialize";
    const unsigned char nestedData[] = {
        0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
    };

    NestedStruct nested = hproto_read<NestedStruct>((char*)nestedData);
    REQUIRE(hproto_size(nested) == sizeof(nestedData));
    REQUIRE(nested.a.a == 3);
    REQUIRE(nested.a.b == 2);
    REQUIRE(nested.a.c == 1);
}
