#include <cassert>
#include <hproto.h>
#include <hlog.h>
#include <variant>

struct StructA {};
HOTSPOT_EMPTY_OBJECT(StructA)
struct StructB {};
HOTSPOT_EMPTY_OBJECT(StructB)
struct StructC {};
HOTSPOT_EMPTY_OBJECT(StructC)

int main() {
    {
        hLog() << "Testing StructA";
        std::variant<StructA> aObj;
        char aReal[hproto_size(aObj)];
        hproto_write(aObj, aReal);

        std::variant readObj = hproto_read<StructA, StructB, StructC>(aReal, hproto_size(aObj));
        assert(std::holds_alternative<StructA>(readObj));
    }
    {
        hLog() << "Testing wrong variant";
        std::variant<StructC> cObj;
        char cReal[hproto_size(cObj)];
        hproto_write(cObj, cReal);

        std::variant readObj = hproto_read<StructA, StructB>(cReal, hproto_size(cObj));
        assert(std::holds_alternative<std::monostate>(readObj));
    }

    return 0;
}
