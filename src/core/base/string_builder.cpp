#include "core/base/string_builder.h"

#include "core/base/string.h"

namespace ark {

StringBuilder::StringBuilder()
    : _dirty(false)
{
}

StringBuilder::StringBuilder(const String& str)
    : _ss(str._str), _dirty(false)
{
}

String StringBuilder::str() const
{
    return _ss.str();
}

void StringBuilder::clear()
{
    _ss.str("");
    _ss.clear();
    _dirty = false;
}

bool StringBuilder::dirty() const
{
    return _dirty;
}

StringBuilder& StringBuilder::operator <<(const String& str)
{
    _ss << str._str;
    _dirty = true;
    return *this;
}

StringBuilder& StringBuilder::operator <<(const char* str)
{
    _ss << str;
    _dirty = true;
    return *this;
}

StringBuilder& StringBuilder::operator <<(int32_t val)
{
    _ss << val;
    _dirty = true;
    return *this;
}

StringBuilder& StringBuilder::operator <<(char c)
{
    _ss << c;
    _dirty = true;
    return *this;
}

StringBuilder& StringBuilder::operator <<(StringBuilder::TypeEndl)
{
    _ss << std::endl;
    _dirty = true;
    return *this;
}

}
