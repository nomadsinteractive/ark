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

const sp<ModelLoader>& WithLayer::modelLoader() const
{
    return _layer_context->modelLoader();
}

TypeId WithLayer:: onWire(WiringContext& context)
{
    if(sp<Renderable> renderable = context.getComponent<Renderable>())
        _layer_context->add(std::move(renderable), context.getComponent<Updatable>(), context.getComponent<Expendable>());
    return TYPE_ID_NONE;
}

}
