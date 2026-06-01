#pragma once

#include <sstream>
#include <iostream>

class HDebug {
    std::ostringstream buffer;

public:
    ~HDebug();

    template<typename T>
    HDebug &operator<<(const T &value) {
        if (!buffer.str().empty())
            buffer << ' ';

        buffer << value;
        return *this;
    }
};

HDebug hLog();
