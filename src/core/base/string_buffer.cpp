#include "core/base/string_buffer.h"

#include "core/base/string.h"

namespace ark {

StringBuffer::StringBuffer()
{
}

StringBuffer::StringBuffer(const String& str)
    : _ss(str._str)
{
}

String StringBuffer::str() const
{
    return _ss.str();
}

void StringBuffer::clear()
{
    _ss.str("");
    _ss.clear();
}

StringBuffer& StringBuffer::operator <<(const String& str)
{
    _ss << str._str;
    return *this;
}

StringBuffer& StringBuffer::operator <<(const char* str)
{
    _ss << str;
    return *this;
}

StringBuffer& StringBuffer::operator <<(uint32_t val)
{
    _ss << val;
    return *this;
}

StringBuffer& StringBuffer::operator <<(int32_t val)
{
    _ss << val;
    return *this;
}

StringBuffer& StringBuffer::operator <<(char c)
{
    _ss << c;
    return *this;
}

StringBuffer& StringBuffer::operator <<(StringBuffer::TypeEndl)
{
    _ss << std::endl;
    return *this;
}

}
