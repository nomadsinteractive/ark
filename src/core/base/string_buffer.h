#pragma once

#include <sstream>

#include "core/base/api.h"

namespace ark {

class ARK_API StringBuffer {
public:
    StringBuffer() = default;

    String str() const;
    void clear();

    StringBuffer& operator <<(StringView str);
    StringBuffer& operator <<(const char* str);
    StringBuffer& operator <<(uint32_t val);
    StringBuffer& operator <<(int32_t val);
    StringBuffer& operator <<(char c);

    typedef decltype(std::endl<char, std::char_traits<char>>) TypeEndl;
    StringBuffer& operator <<(TypeEndl endl);

    bool empty();

private:
    std::ostringstream _ss;
};

}
