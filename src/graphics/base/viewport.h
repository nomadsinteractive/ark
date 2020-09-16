#ifndef ARK_GRAPHICS_BASE_VIEWPORT_H_
#define ARK_GRAPHICS_BASE_VIEWPORT_H_

#include "graphics/base/rect.h"

namespace ark {

class ARK_API Viewport : public Rect {
public:
    Viewport(float left, float top, float right, float bottom, float clipNear, float clipFar);
    Viewport(const Viewport& other);

    float clipNear() const;
    float clipFar() const;

    float toViewportX(float screenX, uint32_t screenWidth) const;
    float toViewportY(float screenY, uint32_t screenHeight) const;

private:
    float _clip_near;
    float _clip_far;

};

}

#endif
