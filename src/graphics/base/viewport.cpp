#include "graphics/base/viewport.h"

namespace ark {

Viewport::Viewport(float left, float top, float right, float bottom, float near, float far)
    : Rect(left, top, right, bottom), _near(near), _far(far)
{
}

Viewport::Viewport(const Viewport& other)
    : Rect(other), _near(other._near), _far(other._far)
{
}

float Viewport::near() const
{
    return _near;
}

float Viewport::far() const
{
    return _far;
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
