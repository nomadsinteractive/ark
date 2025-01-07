#include "core/components/with_tag.h"

#include "core/base/bean_factory.h"

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

void WithTag::onWire(const WiringContext& context, const Box& self)
{
    _with_debris.onWire(context, self);
}

void WithTag::traverse(const Visitor& visitor)
{
    _with_debris.traverse(visitor);
}

WithTag::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _tag(factory.ensureBuilder<Box>(manifest, "tag"))
{
}

sp<Wirable> WithTag::BUILDER::build(const Scope& args)
{
    return sp<Wirable>::make<WithTag>(std::move(*_tag->build(args)));
}

}
