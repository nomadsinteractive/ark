#include "graphics/base/render_command_pipeline.h"

#include "core/types/shared_ptr.h"

namespace ark {

void RenderCommandPipeline::reset()
{
    _commands.clear();
}

void RenderCommandPipeline::add(const sp<RenderCommand>& renderCommand)
{
    if(renderCommand)
        _commands.push_back(renderCommand);
}

void RenderCommandPipeline::draw(GraphicsContext& graphicsContext)
{
    for(const sp<RenderCommand>& i : _commands)
        i->draw(graphicsContext);
}

}
