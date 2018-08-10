#ifndef ARK_GRAPHICS_BASE_V4_H_
#define ARK_GRAPHICS_BASE_V4_H_

#include "core/base/api.h"

#include "graphics/base/v3.h"

namespace ark {

class ARK_API V4 : public V3 {
public:
    V4();
    V4(float x, float y, float z, float w);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(V4);

    bool operator ==(const V4& other) const;
    bool operator !=(const V4& other) const;

    friend V4 operator +(const V4& lvalue, const V4& rvalue);
    friend V4 operator -(const V4& lvalue, const V4& rvalue);
    friend V4 operator *(const V4& lvalue, const V4& rvalue);
    friend V4 operator /(const V4& lvalue, const V4& rvalue);

    float w() const;
    V4 operator -() const;

    float dot(const V4& other) const;

private:
    float _w;

    friend class Color;
};

}

#endif
