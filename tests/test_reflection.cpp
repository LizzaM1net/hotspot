#include <catch2/catch_test_macros.hpp>

#include <hproto.h>
#include <hlog.h>

struct StructA {};
struct StructB {};
struct StructC {};

TEST_CASE("Type name and id are computed correctly for types", "[reflection]") {
    CHECK(HProtoId<StructA>::name == "StructA");
    CHECK(HProtoId<StructA>::id == 4476163269382220915ULL);
    CHECK(HProtoId<StructB>::name == "StructB");
    CHECK(HProtoId<StructB>::id == 4476164368893849126ULL);
    CHECK(HProtoId<StructC>::name == "StructC");
    CHECK(HProtoId<StructC>::id == 4476165468405477337ULL);
    CHECK(HProtoId<std::string>::name == "std::string");
    CHECK(HProtoId<std::string>::id == 2839446397891759191ULL);
    CHECK(HProtoId<double>::name == "double");
    CHECK(HProtoId<double>::id == 11567507311810436776ULL);
    CHECK(HProtoId<std::variant<std::string, double>>::name == "std::variant<std::string, double>");
    CHECK(HProtoId<std::variant<std::string, double>>::id == 9686022488139303836ULL);
}
