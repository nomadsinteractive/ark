#include "graphics/impl/vec/vec2_with_transform.h"

#include "graphics/base/transform.h"
#include "graphics/base/v2.h"

namespace ark {

Vec2WithTransform::Vec2WithTransform(const sp<Vec2>& delegate, const sp<Transform>& transform)
    : _delegate(delegate), _transform(transform)
{
}

V2 Vec2WithTransform::val()
{
    const V2 p = _delegate->val();
    float x, y;
    _transform->snapshot().map(p.x(), p.y(), 0, 0, x, y);
    return V2(x, y);
}

}
