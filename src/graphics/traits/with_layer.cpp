#include "with_layer.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"

namespace ark {

WithLayer::WithLayer(const sp<Layer>& layer)
    : _layer_context(layer->context())
{
}

WithLayer::WithLayer(const sp<RenderLayer>& renderLayer)
    : _layer_context(renderLayer->context())
{
}

WithLayer::WithLayer(sp<LayerContext> layerContext)
    : _layer_context(std::move(layerContext))
{
}

TypeId WithLayer::onPoll(WiringContext& context)
{
    return TYPE_ID_NONE;
}

void WithLayer:: onWire(const WiringContext& context)
{
    if(sp<Renderable> renderable = context.getComponent<Renderable>())
        _layer_context->add(std::move(renderable), context.getComponent<Updatable>(), context.getComponent<Expendable>());
}

const sp<LayerContext>& WithLayer::layerContext() const
{
    return _layer_context;
}

const sp<ModelLoader>& WithLayer::modelLoader() const
{
    return _layer_context->modelLoader();
}

WithLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, Layer::TYPE_UNSPECIFIED))
{
}

sp<Wirable> WithLayer::BUILDER::build(const Scope& args)
{
    return sp<Wirable>::make<WithLayer>(_layer_context->build(args));
}

}
