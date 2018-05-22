#include "graphics/impl/vec/vv2_with_transform.h"

#include "graphics/base/transform.h"
#include "graphics/base/v2.h"

namespace ark {

Vec2WithTransform::Vec2WithTransform(const sp<Vec2>& delegate, const sp<Vec2>& org, const sp<Transform>& transform)
    : _delegate(delegate), _org(org), _transform(transform)
{
}

V2 Vec2WithTransform::val()
{
    const V2 p = _delegate->val();
    const V2 org = _org->val();
    float x, y;
    _transform->snapshot().map(p.x() - org.x(), p.y() - org.y(), org.x(), org.y(), x, y);
    return V2(x, y);
}

}
