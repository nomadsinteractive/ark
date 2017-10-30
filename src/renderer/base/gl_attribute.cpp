#include "renderer/base/gl_attribute.h"

namespace ark {

GLAttribute::GLAttribute()
    : _offset(0), _length(0)
{
}

GLAttribute::GLAttribute(const String& name, const String& type, GLenum glType, uint32_t length, GLboolean normalized)
    : _name(name), _type(type), _gl_type(glType), _offset(0), _length(length), _normalized(normalized)
{
}

GLAttribute::GLAttribute(const GLAttribute& other)
    : _name(other._name), _type(other._type), _gl_type(other._gl_type), _offset(other._offset), _length(other._length), _normalized(other._normalized)
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

void GLAttribute::setVertexPointer(GLuint location, GLsizei stride) const
{
    glVertexAttribPointer(location, _length, _gl_type, _normalized, stride, reinterpret_cast<void*>(_offset));
    glEnableVertexAttribArray(location);
}


}
