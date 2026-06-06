#include "graphics/base/render_request.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/surface_controller.h"

namespace ark {

RenderRequest::RenderRequest(const uint32_t tick)
    : _stub(sp<Stub>::make(tick))
{
}

uint32_t RenderRequest::tick() const
{
    return _stub->_tick;
}

Allocator& RenderRequest::allocator() const
{
    return _stub->_allocator;
}

const sp<RenderCommandPipeline>& RenderRequest::renderCommandPipeLine() const
{
    return _stub->_render_command_pipe_line;
}

void RenderRequest::addRenderCommand(sp<RenderCommand> renderCommand) const
{
    _stub->_render_command_pipe_line->add(std::move(renderCommand));
}

RenderRequest::Stub::Stub(const uint32_t tick)
    : _tick(tick), _render_command_pipe_line(sp<RenderCommandPipeline>::make())
{
}

}
