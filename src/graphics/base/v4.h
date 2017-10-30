#ifndef ARK_GRAPHICS_BASE_V4_H_
#define ARK_GRAPHICS_BASE_V4_H_

#include "core/base/api.h"

#include "graphics/base/v3.h"

namespace ark {

class ARK_API V4 : public V3 {
public:
    V4();
    V4(float x, float y, float z, float w);
    V4(const V4& other);

    bool operator ==(const V4& other) const;
    bool operator !=(const V4& other) const;
    const V4& operator =(const V4& other);

    float w() const;
    void setW(float w);

    float dot(const V4& other) const;

private:
    float _w;
};

}

#endif
