#ifndef ARK_CORE_BASE_STRING_BUFFER_H_
#define ARK_CORE_BASE_STRING_BUFFER_H_

#include <sstream>

#include "core/base/api.h"

namespace ark {

class ARK_API StringBuilder {
public:
    StringBuilder();
    StringBuilder(const String& str);

    String str() const;
    void clear();
    bool dirty() const;

    StringBuilder& operator <<(const String& str);
    StringBuilder& operator <<(const char* str);
    StringBuilder& operator <<(int32_t val);
    StringBuilder& operator <<(char c);

    typedef decltype(std::endl<char, std::char_traits<char>>) TypeEndl;
    StringBuilder& operator <<(TypeEndl endl);

private:
    std::ostringstream _ss;
    bool _dirty;
};

}

#endif
