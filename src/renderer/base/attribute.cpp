#include "renderer/base/attribute.h"

namespace ark {

Attribute::Attribute()
    : _usage(USAGE_CUSTOM), _offset(0), _length(0), _divisor(0)
{
}

Attribute::Attribute(const Usage usage, const String& name, const Type type, const String& declareType, const uint32_t length, const bool normalized)
    : _usage(usage), _name(name), _type(type), _declare_type(declareType), _offset(0), _length(length), _normalized(normalized), _divisor(0)
{
}

const String& Attribute::name() const
{
    return _name;
}

Attribute::Usage Attribute::usage() const
{
    return _usage;
}

Attribute::Type Attribute::type() const
{
    return _type;
}

const String& Attribute::declareType() const
{
    return _declare_type;
}

uint32_t Attribute::offset() const
{
    return _offset;
}

void Attribute::setOffset(const uint32_t offset)
{
    _offset = offset;
}

uint32_t Attribute::divisor() const
{
    return _divisor;
}

void Attribute::setDivisor(const uint32_t divisor)
{
    _divisor = divisor;
}

uint32_t Attribute::length() const
{
    return _length;
}

uint32_t Attribute::componentSize() const
{
    if(_type == TYPE_FLOAT || _type == TYPE_INT || _type == TYPE_UINT)
        return sizeof(float);
    if(_type == TYPE_BYTE || _type == TYPE_UBYTE)
        return 1;
    if(_type == TYPE_SHORT || _type == TYPE_USHORT)
        return sizeof(int16_t);
    FATAL("Unknow component type: %d", _type);
    return 0;
}

uint32_t Attribute::size() const
{
    return componentSize() * _length;
}

bool Attribute::normalized() const
{
    return _normalized;
}

}
