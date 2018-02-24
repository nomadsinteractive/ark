#include "core/base/string_buffer.h"

#include "core/base/string.h"

namespace ark {

StringBuffer::StringBuffer()
    : _dirty(false)
{
}

StringBuffer::StringBuffer(const String& str)
    : _ss(str._str), _dirty(false)
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
    _dirty = false;
}

bool StringBuffer::dirty() const
{
    return _dirty;
}

StringBuffer& StringBuffer::operator <<(const String& str)
{
    _ss << str._str;
    _dirty = true;
    return *this;
}

StringBuffer& StringBuffer::operator <<(const char* str)
{
    _ss << str;
    _dirty = true;
    return *this;
}

StringBuffer& StringBuffer::operator <<(int32_t val)
{
    _ss << val;
    _dirty = true;
    return *this;
}

StringBuffer& StringBuffer::operator <<(char c)
{
    _ss << c;
    _dirty = true;
    return *this;
}

StringBuffer& StringBuffer::operator <<(StringBuffer::TypeEndl)
{
    _ss << std::endl;
    _dirty = true;
    return *this;
}

}
