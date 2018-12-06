#ifndef ARK_RENDERER_BASE_ATTRIBUTE_H_
#define ARK_RENDERER_BASE_ATTRIBUTE_H_

#include "core/base/api.h"
#include "core/base/string.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API Attribute {
public:
    enum Type {
        TYPE_I1,
        TYPE_I2,
        TYPE_I3,
        TYPE_I4,
        TYPE_UI1,
        TYPE_UI2,
        TYPE_UI3,
        TYPE_UI4,
        TYPE_F1,
        TYPE_F2,
        TYPE_F3,
        TYPE_F4,
        TYPE_MAT3,
        TYPE_MAT4
    };

    Attribute();
    Attribute(const String& name, const String& type, GLenum glType, uint32_t length, bool normalized);
    DEFAULT_COPY_AND_ASSIGN(Attribute);

    const String& name() const;
    const String& type() const;

    uint32_t offset() const;
    void setOffset(uint32_t offset);

    uint32_t divisor() const;
    void setDivisor(uint32_t divisor);

    uint32_t length() const;
    uint32_t size() const;

    void setVertexPointer(GLint location, GLsizei stride) const;

private:
    void setVertexPointer(GLint location, GLsizei stride, uint32_t length, uint32_t offset, bool normalized, uint32_t divisor) const;

private:
    String _name;
    String _type;
    GLenum _gl_type;

    uint32_t _offset;
    uint32_t _length;
    bool _normalized;

    uint32_t _divisor;
};

}

#endif
