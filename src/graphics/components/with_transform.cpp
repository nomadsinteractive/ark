#include "graphics/components/with_transform.h"

namespace ark {

WithTransform::WithTransform(sp<Transform> transform)
    : _transform(std::move(transform))
{
}

TypeId WithTransform::onPoll(WiringContext& context)
{
    context.setComponent(_transform);
    return constants::TYPE_ID_NONE;
}

void WithTransform::onWire(const WiringContext& context)
{
}

}