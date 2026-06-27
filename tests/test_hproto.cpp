#include <cassert>
#include <cstdint>
#include <hproto.h>
#include <iostream>
#include <iomanip>

template<typename T>
requires std::is_aggregate_v<T>
void print_struct_bytes(const T& obj) {
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&obj);
    std::size_t size = sizeof(T);

    std::cout << "--- Raw Hex Dump (" << size << " bytes) ---" << std::hex;

    for (std::size_t i = 0; i < size; ++i) {
        if (i % 16 == 0)
            std::cout << std::endl;

        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}

struct SimpleStruct {
    int32_t a, b, c;
};
HOTSPOT_SIZED_OBJECT(SimpleStruct, 0x01, 12)

struct PaddedStruct {
    int32_t a;
    double b;
    int32_t c;
};
HOTSPOT_SIZED_OBJECT(PaddedStruct, 0x02, 24)

int main() {
    SimpleStruct simpleObj{1, 2, 3};
    print_struct_bytes(simpleObj);
    const char simpleExpected[] = {
        0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
    };
    char simpleReal[sizeof(SimpleStruct)];
    HProtoData<SimpleStruct>::hproto_write(simpleObj, simpleReal);
    assert(memcmp(&simpleObj, simpleExpected, sizeof(SimpleStruct)) == 0);

    PaddedStruct paddedObj;
    // Immitates uninitialized data before constructor run
    std::memset(&paddedObj, 0xfe, sizeof(PaddedStruct));
    std::construct_at(&paddedObj, 1, 2.0, 3);
    print_struct_bytes(paddedObj);
    const char paddedExpected[] = {
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    char paddedReal[sizeof(PaddedStruct)];
    HProtoData<PaddedStruct>::hproto_write(paddedObj, paddedReal);
    assert(memcmp(paddedReal, paddedExpected, sizeof(PaddedStruct)) == 0);
    return 0;
}
