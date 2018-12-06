#include "renderer/base/attribute.h"

namespace ark {

Attribute::Attribute()
    : _offset(0), _length(0), _divisor(0)
{
}

Attribute::Attribute(const String& name, const String& type, GLenum glType, uint32_t length, bool normalized)
    : _name(name), _type(type), _gl_type(glType), _offset(0), _length(length), _normalized(normalized), _divisor(0)
{
}

const String& Attribute::name() const
{
    return _name;
}

const String& Attribute::type() const
{
    return _type;
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

uint32_t Attribute::size() const
{
    if(_gl_type == GL_FLOAT || _gl_type == GL_INT || _gl_type == GL_UNSIGNED_INT)
        return _length * sizeof(GLfloat);
    if(_gl_type == GL_BYTE || _gl_type == GL_UNSIGNED_BYTE)
        return _length;
    if(_gl_type == GL_SHORT || _gl_type == GL_UNSIGNED_SHORT)
        return _length * sizeof(int16_t);
    DFATAL("Unimplemented");
    return 0;
}

void Attribute::setVertexPointer(GLint location, GLsizei stride) const
{
    DWARN(location >= 0, "Attribute \"%s\" location: %d", _name.c_str(), location);
    if(_length <= 4)
        setVertexPointer(location, stride, _length, _offset, _normalized, _divisor);
    else if(_length == 16)
    {
        uint32_t offset = size() / 4;
        for(uint32_t i = 0; i < 4; i++)
            setVertexPointer(location + i, stride, 4, _offset + offset * i, _normalized, _divisor);
    }
    else if(_length == 9)
    {
        uint32_t offset = size() / 3;
        for(uint32_t i = 0; i < 3; i++)
            setVertexPointer(location + i, stride, 3, _offset + offset * i, _normalized, _divisor);
    }
}

void Attribute::setVertexPointer(GLint location, GLsizei stride, uint32_t length, uint32_t offset, bool normalized, uint32_t divisor) const
{
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, length, _gl_type, normalized ? GL_TRUE : GL_FALSE, stride, reinterpret_cast<void*>(offset));
    if(divisor)
        glVertexAttribDivisor(location, divisor);
}


}
