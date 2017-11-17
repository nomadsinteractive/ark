#include "graphics/inf/layer.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_request.h"

namespace ark {

Layer::Layer()
    : _layer_context(sp<LayerContext>::make())
{
}

const sp<LayerContext>& Layer::layerContext() const
{
    return _layer_context;
}

Layer::Renderer::Renderer(const sp<Layer>& layer)
    : _layer(layer)
{
}

void Layer::Renderer::render(RenderRequest& renderRequest, float x, float y)
{
/*
    const sp<RenderCommand> renderCommand = _layer->render(_layer->layerContext()->snapshot(), x, y);
    _layer->layerContext()->clear();
    if(renderCommand)
        renderRequest.addRequest(renderCommand);
/*/
    renderRequest.addBackgroundRequest(_layer, x, y);
/**/
}

}
