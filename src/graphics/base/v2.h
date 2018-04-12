#ifndef ARK_GRAPHICS_BASE_V2_H_
#define ARK_GRAPHICS_BASE_V2_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API V2 {
public:
    V2();
    V2(float x, float y);
    V2(float x, float y, float z);
    V2(const V2& other);
    V2(const V3& other);

    static V2 identity();

    bool operator ==(const V2& other) const;
    bool operator !=(const V2& other) const;
    const V2& operator =(const V2& other);

    friend V2 operator +(const V2& lvalue, const V2& rvalue);
    friend V2 operator -(const V2& lvalue, const V2& rvalue);
    friend V2 operator *(const V2& lvalue, const V2& rvalue);
    friend V2 operator /(const V2& lvalue, const V2& rvalue);


    float x() const;
    float y() const;
    float z() const;

    float dot(const V2& other) const;

private:
    float _x;
    float _y;

    friend class V3;
    friend class V4;
    friend class Color;
};

}

#endif
