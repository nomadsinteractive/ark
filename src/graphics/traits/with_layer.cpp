#include "with_layer.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"

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

std::vector<std::pair<TypeId, Box>> WithLayer::onWire(const Traits& components)
{
    if(sp<Renderable> renderable = components.get<Renderable>())
        _layer_context->add(std::move(renderable), components.get<Updatable>(), components.get<Disposed>());
    return {};
}

}