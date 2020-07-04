#include "graphics/base/render_command_pipeline.h"

#include "core/types/shared_ptr.h"

namespace ark {

void RenderCommandPipeline::add(sp<RenderCommand> renderCommand)
{
    if(renderCommand)
        _commands.push_back(std::move(renderCommand));
}

void RenderCommandPipeline::draw(GraphicsContext& graphicsContext)
{
    for(const sp<RenderCommand>& i : _commands)
        i->draw(graphicsContext);
}

}
