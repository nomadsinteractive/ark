#include "graphics/inf/layer.h"

#include "graphics/base/layer_context.h"

namespace ark {

Layer::Layer()
    : _render_context(sp<LayerContext>::make())
{
}

const sp<LayerContext>& Layer::renderContext() const
{
    return _render_context;
}

void Layer::render(RenderCommandPipeline& pipeline, float x, float y)
{
    render(_render_context, pipeline, x, y);
    _render_context->clear();
}

}
