#include <hproto.h>
#include <hlog.h>

struct StructA {};
struct StructB {};
struct StructC {};

int main() {
    assert(HProtoId<StructA>::id == 4476163269382220915ULL);
    assert(HProtoId<StructA>::name == "StructA");
    assert(HProtoId<StructB>::id == 4476164368893849126ULL);
    assert(HProtoId<StructB>::name == "StructB");
    assert(HProtoId<StructC>::id == 4476165468405477337ULL);
    assert(HProtoId<StructC>::name == "StructC");
    assert(HProtoId<std::string>::id == 2839446397891759191ULL);
    assert(HProtoId<std::string>::name == "std::string");
    assert(HProtoId<double>::id == 11567507311810436776ULL);
    assert(HProtoId<double>::name == "double");
    assert((HProtoId<std::variant<std::string, double>>::id == 9686022488139303836ULL));
    assert((HProtoId<std::variant<std::string, double>>::name == "std::variant<std::string, double>"));

    return 0;
}
