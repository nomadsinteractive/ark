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
    const V3 p = _transform->snapshot().transform(V3(_delegate->val(), 0));
    return V2(p.x(), p.y());
}

}
