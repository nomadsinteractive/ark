#include "with_layer.h"

#include "core/base/constants.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"

namespace ark {

WithLayer::WithLayer(sp<Layer> layer)
    : _layer(std::move(layer))
{
}

TypeId WithLayer::onPoll(WiringContext& context)
{
    return constants::TYPE_ID_NONE;
}

void WithLayer::onWire(const WiringContext& context)
{
    if(sp<Renderable> renderable = context.getComponent<Renderable>())
        _layer->add(std::move(renderable), context.getComponent<Updatable>(), context.getComponent<Discarded>());
}

const sp<Layer>& WithLayer::layer() const
{
    return _layer;
}

WithLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _layer(factory.ensureBuilder<Layer>(manifest, constants::LAYER))
{
}

sp<Wirable> WithLayer::BUILDER::build(const Scope& args)
{
    return sp<Wirable>::make<WithLayer>(_layer->build(args));
}

}
