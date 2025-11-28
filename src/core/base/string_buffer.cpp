#include "core/base/string_buffer.h"

#include "core/base/string.h"

namespace ark {

String StringBuffer::str() const
{
    return {_ss.str()};
}

void StringBuffer::clear()
{
    _ss = {};
}

StringBuffer& StringBuffer::operator<<(const StringView str)
{
    _ss << str;
    return *this;
}

StringBuffer& StringBuffer::operator <<(const char* str)
{
    _ss << str;
    return *this;
}

StringBuffer& StringBuffer::operator <<(const uint32_t val)
{
    _ss << val;
    return *this;
}

StringBuffer& StringBuffer::operator <<(const int32_t val)
{
    _ss << val;
    return *this;
}

StringBuffer& StringBuffer::operator <<(const char c)
{
    _ss << c;
    return *this;
}

StringBuffer& StringBuffer::operator <<(StringBuffer::TypeEndl)
{
    _ss << std::endl;
    return *this;
}

bool StringBuffer::empty()
{
    return _ss.tellp() == 0;
}

}
