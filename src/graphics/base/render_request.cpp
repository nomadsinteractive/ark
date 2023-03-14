#include "graphics/base/render_request.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/surface_controller.h"
#include "graphics/inf/render_view.h"

namespace ark {

RenderRequest::RenderRequest(uint64_t timestamp, sp<Allocator::Pool> allocatorPool)
    : _stub(sp<Stub>::make(timestamp, std::move(allocatorPool)))
{
}

uint64_t RenderRequest::timestamp() const
{
    return _stub->_timestamp;
}

Allocator& RenderRequest::allocator() const
{
    return _stub->_allocator;
}

void RenderRequest::onRenderFrame(const Color& backgroundColor, RenderView& renderView) const
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

RenderRequest::Stub::Stub(uint64_t timestamp, sp<Allocator::Pool> allocatorPool)
    : _timestamp(timestamp), _allocator(std::move(allocatorPool)), _render_command_pipe_line(sp<RenderCommandPipeline>::make())
{
}

}
