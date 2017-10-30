#ifndef ARK_GRAPHICS_BASE_V2_H_
#define ARK_GRAPHICS_BASE_V2_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API V2 {
public:
    V2();
    V2(float x, float y);
    V2(const V2& other);

    bool operator ==(const V2& other) const;
    bool operator !=(const V2& other) const;
    const V2& operator =(const V2& other);

    float x() const;
    void setX(float x);

    float y() const;
    void setY(float y);

    float dot(const V2& other) const;

private:
    float _x;
    float _y;

    friend class V3;
    friend class V4;
};

}

#endif
