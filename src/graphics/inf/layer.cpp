#include "graphics/inf/layer.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"

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
    const sp<RenderCommand> renderCommand = render(_render_context, x, y);
    _render_context->clear();
    if(renderCommand)
        pipeline.add(renderCommand);
}

}
