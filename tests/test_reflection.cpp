#include <catch2/catch_test_macros.hpp>

#include <hproto.h>
#include <hbuffer.h>
#include <hlog.h>

struct StructA {};
struct StructB {};
struct StructC {};

TEST_CASE("Type name and id are computed correctly for types", "[reflection]") {
    CHECK(hproto_name<StructA>() == "StructA");
    CHECK(hproto_id<StructA>() == 4476163269382220915ULL);
    CHECK(hproto_name<StructB>() == "StructB");
    CHECK(hproto_id<StructB>() == 4476164368893849126ULL);
    CHECK(hproto_name<StructC>() == "StructC");
    CHECK(hproto_id<StructC>() == 4476165468405477337ULL);
    CHECK(hproto_name<std::string>() == "HString");
    CHECK(hproto_id<std::string>() == 15590282732832896450ULL);
    CHECK(hproto_name<double>() == "double");
    CHECK(hproto_id<double>() == 11567507311810436776ULL);
    // CHECK(HProtoId<std::variant<std::string, double>>::name == "std::variant<std::string, double>");
    // CHECK(HProtoId<std::variant<std::string, double>>::id == 9686022488139303836ULL);
}
