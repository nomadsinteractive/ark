#ifndef ARK_RENDERER_BASE_GL_ATTRIBUTE_H_
#define ARK_RENDERER_BASE_GL_ATTRIBUTE_H_

#include "core/base/api.h"
#include "core/base/string.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLAttribute {
public:
    GLAttribute();
    GLAttribute(const String& name, const String& type, GLenum glType, uint32_t length, GLboolean normalized);
    GLAttribute(const GLAttribute& other) = default;

    const String& name() const;
    const String& type() const;

    uint32_t offset() const;
    void setOffset(uint32_t offset);

    uint32_t divisor() const;
    void setDivisor(uint32_t divisor);

    uint32_t length() const;
    uint32_t size() const;

    void setVertexPointer(GLuint location, GLsizei stride) const;

private:
    void setVertexPointer(GLuint location, GLsizei stride, uint32_t length, uint32_t offset) const;

private:
    String _name;
    String _type;
    GLenum _gl_type;

    uint32_t _offset;
    uint32_t _length;
    GLboolean _normalized;

    uint32_t _divisor;
};

}

#endif
