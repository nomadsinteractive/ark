#include "graphics/impl/vec/vec2_with_transform.h"

#include "core/util/updatable_util.h"

#include "graphics/inf/transform.h"
#include "graphics/base/v2.h"

namespace ark {

Vec2WithTransform::Vec2WithTransform(const sp<Vec2>& delegate, const sp<Transform>& transform)
    : _delegate(delegate), _transform(transform)
{
}

V2 Vec2WithTransform::val()
{
    const V3 transformed = _transform->transform(_transform->snapshot(), {_delegate->val(), 0, 1.0f}).toNonHomogeneous();
    return {transformed.x(), transformed.y()};
}

bool Vec2WithTransform::update(uint64_t timestamp)
{
    return UpdatableUtil::update(timestamp, _delegate, _transform);
}

}
