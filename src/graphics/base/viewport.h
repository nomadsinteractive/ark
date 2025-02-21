#pragma once

#include "graphics/base/rect.h"

namespace ark {

class ARK_API Viewport : public Rect {
public:
    Viewport(float left, float top, float right, float bottom, float clipNear, float clipFar);
    Viewport(const Viewport& other);

    float clipNear() const;
    float clipFar() const;

    float toViewportX(float screenX, float surfaceWidth) const;
    float toViewportY(float screenY, float surfaceHeight, bool flip = false) const;

private:
    float _clip_near;
    float _clip_far;

};

}
