#include "app/components/with_tag.h"

namespace ark {

WithTag::WithTag(Box tag)
{
    setTag(std::move(tag));
}

const Box& WithTag::tag() const
{
    return _tag;
}

void WithTag::setTag(Box tag)
{
    _with_debris.track(tag);
    _tag = std::move(tag);
}

void WithTag::onWire(const WiringContext& context)
{
    _with_debris.onWire(context);
}

void WithTag::traverse(const Visitor& visitor)
{
    _with_debris.traverse(visitor);
}

}
