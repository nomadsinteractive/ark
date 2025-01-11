#include "graphics/base/viewport.h"

#include "core/util/math.h"

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

float Viewport::toViewportX(float screenX, float surfaceWidth) const
{
    return Math::lerp(left(), right(), screenX / surfaceWidth);
}

float Viewport::toViewportY(float screenY, float surfaceHeight) const
{
    return Math::lerp(top(), bottom(), screenY / surfaceHeight);
}

}
