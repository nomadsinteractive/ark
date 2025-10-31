#include "graphics/base/render_request.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/surface_controller.h"
#include "graphics/inf/render_view.h"

namespace ark {

RenderRequest::RenderRequest(const uint32_t tick, sp<Allocator::Pool> allocatorPool)
    : _stub(sp<Stub>::make(tick, std::move(allocatorPool)))
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

void RenderRequest::onRenderFrame(const V4 backgroundColor, RenderView& renderView) const
{
    renderView.onRenderFrame(backgroundColor, _stub->_render_command_pipe_line);
}

RenderRequest::RenderRequest(sp<RenderRequest::Stub> stub)
    : _stub(std::move(stub))
{
}

void RenderRequest::addRenderCommand(sp<RenderCommand> renderCommand) const
{
    _stub->_render_command_pipe_line->add(std::move(renderCommand));
}

RenderRequest::Stub::Stub(const uint32_t tick, sp<Allocator::Pool> allocatorPool)
    : _tick(tick), _allocator(std::move(allocatorPool)), _render_command_pipe_line(sp<RenderCommandPipeline>::make())
{
}

}
