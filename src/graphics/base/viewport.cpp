#include "graphics/base/viewport.h"

namespace ark {

Viewport::Viewport(float left, float top, float right, float bottom, float clipNear, float clipFar)
    : Rect(left, top, right, bottom), _clip_near(clipNear), _clip_far(clipFar)
{
}

Viewport::Viewport(const Viewport& other)
    : Rect(other), _clip_near(other._clip_near), _clip_far(other._clip_far)
{
}

float Viewport::clipNear() const
{
    return _clip_near;
}

float Viewport::clipFar() const
{
    return _clip_far;
}

float Viewport::toViewportX(float screenX, uint32_t screenWidth) const
{
    return left() + (right() - left()) * screenX / screenWidth;
}

float Viewport::toViewportY(float screenY, uint32_t screenHeight) const
{
    const float bt = bottom() - top();
    return bt > 0.0f ? bt - bt * screenY / screenHeight : - bt * screenY / screenHeight;
}

}
