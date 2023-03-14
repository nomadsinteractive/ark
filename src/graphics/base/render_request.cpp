#include "graphics/base/render_request.h"

#include "core/inf/executor.h"
#include "core/inf/runnable.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/surface_controller.h"
#include "graphics/base/render_layer_snapshot.h"
#include "graphics/inf/render_view.h"

namespace ark {

namespace {

class BackgroundRenderCommand : public RenderCommand, public Runnable {
public:
    BackgroundRenderCommand(const RenderRequest& renderRequest, const RenderLayer& layer, const V3& position)
        : _render_request(renderRequest), _layer_snapshot(layer.snapshot(_render_request)), _position(position) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _delegate->draw(graphicsContext);
    }

    virtual void run() override {
        _delegate = _layer_snapshot.render(_render_request, _position);
        DASSERT(_delegate);
        _render_request.jobDone();
    }

private:
    RenderRequest _render_request;
    RenderLayerSnapshot _layer_snapshot;
    V3 _position;

    sp<RenderCommand> _delegate;
};

}

RenderRequest::RenderRequest(uint64_t timestamp, sp<Allocator::Pool> allocatorPool, sp<Executor> executor, sp<OCSQueue<RenderRequest>> renderRequests)
    : _stub(sp<Stub>::make(timestamp, std::move(allocatorPool), std::move(executor), std::move(renderRequests)))
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

void RenderRequest::jobDone()
{
    _stub->onJobDone(_stub);
    _stub = nullptr;
}

void RenderRequest::addRequest(sp<RenderCommand> renderCommand) const
{
    _stub->_render_command_pipe_line->add(std::move(renderCommand));
}

void RenderRequest::addBackgroundRequest(const RenderLayer& renderLayer, const V3& position)
{
    sp<BackgroundRenderCommand> renderCommand = sp<BackgroundRenderCommand>::make(*this, renderLayer, position);
    ++(_stub->_background_renderer_count);
    renderLayer.executor()->execute(renderCommand);
    _stub->_render_command_pipe_line->add(std::move(renderCommand));
}

RenderRequest::Stub::Stub(uint64_t timestamp, sp<Allocator::Pool> allocatorPool, sp<Executor> executor, sp<OCSQueue<RenderRequest>> renderRequests)
    : _timestamp(timestamp), _allocator(std::move(allocatorPool)), _executor(std::move(executor)), _render_requests(std::move(renderRequests)), _render_command_pipe_line(sp<RenderCommandPipeline>::make()), _background_renderer_count(1)
{
}

void RenderRequest::Stub::onJobDone(const sp<Stub>& self)
{
    int32_t count = --_background_renderer_count;
    DCHECK(count >= 0, "Bad count: %d", count);
    if(count == 0)
        _render_requests->add(RenderRequest(self));
}

}
