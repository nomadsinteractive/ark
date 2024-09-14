#include "graphics/traits/with_transform.h"

namespace ark {
WithTransform::WithTransform(sp<Mat4> transform)
    : _transform(std::move(transform))
{
}

TypeId WithTransform::onPoll(WiringContext& context)
{
    return constants::TYPE_ID_NONE;
}

void WithTransform::onWire(const WiringContext& context)
{
}

const sp<Mat4>& WithTransform::transform() const
{
    return _transform;
}

}
