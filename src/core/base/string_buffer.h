#ifndef ARK_CORE_BASE_STRING_BUFFER_H_
#define ARK_CORE_BASE_STRING_BUFFER_H_

#include <sstream>

#include "core/base/api.h"

namespace ark {

class ARK_API StringBuffer {
public:
    StringBuffer();
    StringBuffer(const String& str);

    String str() const;
    void clear();

    StringBuffer& operator <<(const String& str);
    StringBuffer& operator <<(const char* str);
    StringBuffer& operator <<(uint32_t val);
    StringBuffer& operator <<(int32_t val);
    StringBuffer& operator <<(char c);

    typedef decltype(std::endl<char, std::char_traits<char>>) TypeEndl;
    StringBuffer& operator <<(TypeEndl endl);

private:
    std::ostringstream _ss;
};

}

#endif
