#include "graphics/inf/layer.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_request.h"

namespace ark {

Layer::Layer(const sp<MemoryPool>& memoryPool)
    : _layer_context(sp<LayerContext>::make(memoryPool))
{
}

const sp<LayerContext>& Layer::layerContext() const
{
    return _layer_context;
}

Layer::Renderer::Renderer(const sp<Layer>& layer)
    : _layer(layer)
{
    NOT_NULL(_layer);
}

void Layer::Renderer::render(RenderRequest& renderRequest, float x, float y)
{
    renderRequest.addBackgroundRequest(_layer, x, y);
}

}
