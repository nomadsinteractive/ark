#include "renderer/base/gl_attribute.h"

namespace ark {

GLAttribute::GLAttribute()
    : _offset(0), _length(0), _divisor(0)
{
}

GLAttribute::GLAttribute(const String& name, const String& type, GLenum glType, uint32_t length, GLboolean normalized)
    : _name(name), _type(type), _gl_type(glType), _offset(0), _length(length), _normalized(normalized), _divisor(0)
{
}

const String& GLAttribute::name() const
{
    return _name;
}

const String& GLAttribute::type() const
{
    return _type;
}

uint32_t GLAttribute::offset() const
{
    return _offset;
}

void GLAttribute::setOffset(uint32_t offset)
{
    _offset = offset;
}

uint32_t GLAttribute::divisor() const
{
    return _divisor;
}

void GLAttribute::setDivisor(uint32_t divisor)
{
    _divisor = divisor;
}

uint32_t GLAttribute::length() const
{
    return _length;
}

uint32_t GLAttribute::size() const
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

void GLAttribute::setVertexPointer(GLint location, GLsizei stride) const
{
    DWARN(location >= 0, "Attribute \"%s\" location: %d", _name.c_str(), location);
    if(_length <= 4)
        setVertexPointer(location, stride, _length, _offset);
    else if(_length == 16)
    {
        uint32_t offset = size() / 4;
        for(uint32_t i = 0; i < 4; i++)
            setVertexPointer(location + i, stride, 4, _offset + offset * i);
    }
    else if(_length == 9)
    {
        uint32_t offset = size() / 3;
        for(uint32_t i = 0; i < 3; i++)
            setVertexPointer(location + i, stride, 3, _offset + offset * i);
    }
}

void GLAttribute::setVertexPointer(GLint location, GLsizei stride, uint32_t length, uint32_t offset) const
{
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, length, _gl_type, _normalized, stride, reinterpret_cast<void*>(offset));
    if(_divisor)
        glVertexAttribDivisor(location, _divisor);
}


}
