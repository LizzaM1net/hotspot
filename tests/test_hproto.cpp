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
    char numberReal[hproto_traits<int>::size()];
    hproto_write(number, numberReal);
    REQUIRE(hproto_traits<int>::size() == 4);
    REQUIRE(memcmp(numberReal, numberExpected, hproto_traits<int>::size()) == 0);
}

TEST_CASE("hproto_read deserializes int correctly", "[hproto]") {
    hLog() << "Testing int deserialize";
    const unsigned char numberData[] = {
        0x0A, 0x00, 0x00, 0x00,
    };
    REQUIRE(sizeof(numberData) == hproto_traits<int>::size());

    int number = hproto_read<int>((char *)numberData);
    REQUIRE(number == 10);
}

TEST_CASE("hproto_write serializes double correctly", "[hproto]") {
    hLog() << "Testing double serialize";
    double number = 1.42;
    const unsigned char numberExpected[] = {
        0xB8, 0x1E, 0x85, 0xEB,
        0x51, 0xB8, 0xF6, 0x3F,
    };
    char numberReal[hproto_traits<double>::size()];
    hproto_write(number, numberReal);
    REQUIRE(hproto_traits<double>::size() == 8);
    REQUIRE(memcmp(numberReal, numberExpected, hproto_traits<double>::size()) == 0);
}

TEST_CASE("hproto_write deserializes double correctly", "[hproto]") {
    hLog() << "Testing double deserialize";
    const unsigned char numberData[] = {
        0xB8, 0x1E, 0x85, 0xEB,
        0x51, 0xB8, 0xF6, 0x3F,
    };
    REQUIRE(sizeof(numberData) == hproto_traits<double>::size());

    double number = hproto_read<double>((char*)numberData);
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
    char simpleReal[hproto_traits<SimpleStruct>::size()];
    hproto_write(simpleObj, simpleReal);
    REQUIRE(hproto_traits<SimpleStruct>::size() == 12);
    REQUIRE(memcmp(simpleReal, simpleExpected, hproto_traits<SimpleStruct>::size()) == 0);
}

TEST_CASE("hproto_write deserializes SimpleStruct correctly", "[hproto]") {
    hLog() << "Testing SimpleStruct deserialize";
    const unsigned char simpleData[] = {
        0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
    };
    REQUIRE(sizeof(simpleData) == hproto_traits<SimpleStruct>::size());

    SimpleStruct simple = hproto_read<SimpleStruct>((char*)simpleData);
    REQUIRE(simple.a == 1);
    REQUIRE(simple.b == 2);
    REQUIRE(simple.c == 3);
}

TEST_CASE("hproto_write serializes PaddedStruct correctly, zeroing padding", "[hproto]") {
    hLog() << "Testing PaddedStruct serialize";
    PaddedStruct paddedObj;
    // Immitates uninitialized data before constructor run
    std::memset(&paddedObj, 0xfe, sizeof(PaddedStruct));
    std::construct_at(&paddedObj, 1, 2.0, 3);
    const unsigned char paddedExpected[] = {
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    char paddedReal[hproto_traits<PaddedStruct>::size()];
    hproto_write(paddedObj, paddedReal);
    REQUIRE(hproto_traits<PaddedStruct>::size() == 24);
    REQUIRE(memcmp(paddedReal, paddedExpected, hproto_traits<PaddedStruct>::size()) == 0);
}

TEST_CASE("hproto_write deserializes PaddedStruct correctly, zeroing padding", "[hproto]") {
    hLog() << "Testing PaddedStruct deserialize";
    // Compare data with garbage in the padding against clean values
    const unsigned char paddedData[] = {
        0x01, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFE, 0xFE,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
        0x03, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFE, 0xFE,
    };
    REQUIRE(sizeof(paddedData) == hproto_traits<PaddedStruct>::size());

    PaddedStruct padded = hproto_read<PaddedStruct>((char*)paddedData);
    REQUIRE(padded.a == 1);
    REQUIRE(padded.b == 2.0);
    REQUIRE(padded.c == 3);
}

TEST_CASE("hproto_write serializes EmptyStruct correctly", "[hproto]") {
    hLog() << "Testing EmptyStruct serialize";
    EmptyStruct emptyObj;
    const unsigned char emptyExpected[] = {
        0x00
    };
    char emptyReal[sizeof(EmptyStruct)];
    hproto_write(emptyObj, emptyReal);
    REQUIRE(hproto_traits<EmptyStruct>::size() == 1);
    REQUIRE(memcmp(emptyReal, emptyExpected, sizeof(EmptyStruct)) == 0);
}

TEST_CASE("hproto_write deserializes EmptyStruct correctly", "[hproto]") {
    hLog() << "Testing EmptyStruct deserialize";
    const unsigned char emptyData[] = {
        0x00
    };
    REQUIRE(sizeof(emptyData) == hproto_traits<EmptyStruct>::size());

    EmptyStruct empty = hproto_read<EmptyStruct>((char*)emptyData);
}

TEST_CASE("hproto_write serializes NestedStruct correctly", "[hproto]") {
    hLog() << "Testing NestedStruct serialize";
    NestedStruct nestedObj{
        {3, 2, 1}
    };
    const unsigned char nestedExpected[] = {
        0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    char nestedReal[sizeof(NestedStruct)];
    hproto_write(nestedObj, nestedReal);
    REQUIRE(hproto_traits<NestedStruct>::size() == 16);
    REQUIRE(memcmp(nestedReal, nestedExpected, sizeof(NestedStruct)) == 0);
}

TEST_CASE("hproto_write deserializes NestedStruct correctly", "[hproto]") {
    hLog() << "Testing NestedStruct deserialize";
    const unsigned char nestedData[] = {
        0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    REQUIRE(sizeof(nestedData) == hproto_traits<NestedStruct>::size());

    NestedStruct nestedObj{
        {3, 2, 1}
    };
    NestedStruct nested = hproto_read<NestedStruct>((char*)nestedData);
    REQUIRE(nested.a.a == 3);
    REQUIRE(nested.a.b == 2);
    REQUIRE(nested.a.c == 1);
}
