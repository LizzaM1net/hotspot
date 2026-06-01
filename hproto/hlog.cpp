#include "hlog.h"

HDebug::~HDebug() {
    std::clog << buffer.str() << std::endl;
}

HDebug hLog() {
    return HDebug();
}
