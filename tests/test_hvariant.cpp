#include <catch2/catch_test_macros.hpp>

#include <cassert>
#include <hproto.h>
#include <hlog.h>
#include <variant>

struct StructA {};
struct StructB {};
struct StructC {};

TEST_CASE("HVariant unpacks packed variant correctly", "[hvariant]") {
    hLog() << "Testing StructA";
    std::variant<StructA> aObj;
    char aReal[hproto_size(aObj)];
    hproto_write(aObj, aReal);

    std::variant readObj = hproto_read<StructA, StructB, StructC>(aReal, hproto_size(aObj));
    CHECK(std::holds_alternative<StructA>(readObj));
}

TEST_CASE("HVariant falls back to std::monostate for unknown types", "[hvariant]") {
    hLog() << "Testing wrong variant";
    std::variant<StructC> cObj;
    char cReal[hproto_size(cObj)];
    hproto_write(cObj, cReal);

    std::variant readObj = hproto_read<StructA, StructB>(cReal, hproto_size(cObj));
    CHECK(std::holds_alternative<std::monostate>(readObj));
}
