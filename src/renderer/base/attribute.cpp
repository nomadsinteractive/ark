#include "renderer/base/attribute.h"

namespace ark {

Attribute::Attribute()
    : _offset(0), _length(0), _divisor(0)
{
}

Attribute::Attribute(const String& name, Type type, const String& declareType, uint32_t length, bool normalized)
    : _name(name), _type(type), _declare_type(declareType), _offset(0), _length(length), _normalized(normalized), _divisor(0)
{
}

const String& Attribute::name() const
{
    return _name;
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

void Attribute::setOffset(uint32_t offset)
{
    _offset = offset;
}

uint32_t Attribute::divisor() const
{
    return _divisor;
}

void Attribute::setDivisor(uint32_t divisor)
{
    _divisor = divisor;
}

uint32_t Attribute::length() const
{
    return _length;
}

uint32_t Attribute::componentSize() const
{
    if(_type == TYPE_FLOAT || _type == TYPE_INTEGER)
        return sizeof(float);
    if(_type == TYPE_BYTE || _type == TYPE_UBYTE)
        return 1;
    if(_type == TYPE_SHORT || _type == TYPE_USHORT)
        return sizeof(int16_t);
    FATAL("Unknow compoent type: %d", _type);
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
