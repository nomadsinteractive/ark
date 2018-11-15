#ifndef ARK_GRAPHICS_BASE_VIEWPORT_H_
#define ARK_GRAPHICS_BASE_VIEWPORT_H_

#include "graphics/base/rect.h"

// thanks for reminding me the good old days programming in MSDOS
#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

namespace ark {

class ARK_API Viewport : public Rect {
public:
    Viewport(float left, float top, float right, float bottom, float near, float far);
    Viewport(const Viewport& other);

    float near() const;
    float far() const;

    float toViewportX(float screenX, uint32_t screenWidth);
    float toViewportY(float screenY, uint32_t screenHeight);

private:
    float _near;
    float _far;

};

}

#endif
