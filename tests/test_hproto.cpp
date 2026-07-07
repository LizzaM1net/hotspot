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

int main() {
    {
        hLog() << "Testing SimpleStruct";
        SimpleStruct simpleObj{1, 2, 3};
        const char simpleExpected[] = {
            0x01, 0x00, 0x00, 0x00,
            0x02, 0x00, 0x00, 0x00,
            0x03, 0x00, 0x00, 0x00,
        };
        char simpleReal[sizeof(SimpleStruct)];
        hproto_write(simpleObj, simpleReal);
        assert(memcmp(simpleReal, simpleExpected, sizeof(SimpleStruct)) == 0);
    }

    {
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
        assert(memcmp(paddedReal, paddedExpected, sizeof(PaddedStruct)) == 0);
    }

    {
        hLog() << "Testing EmptyStruct";
        EmptyStruct emptyObj;
        const char emptyExpected[] = {
            0x00
        };
        char emptyReal[sizeof(EmptyStruct)];
        hproto_write(emptyObj, emptyReal);
        assert(memcmp(emptyReal, emptyExpected, sizeof(EmptyStruct)) == 0);
    }

    return 0;
}
