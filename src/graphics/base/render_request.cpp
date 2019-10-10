#include "graphics/base/render_request.h"

#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/impl/runnable/runnable_with_callback.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/surface_controller.h"
#include "graphics/base/render_layer.h"

namespace ark {

namespace {

class BackgroundRenderCommand : public RenderCommand, public Runnable {
public:
    BackgroundRenderCommand(const RenderRequest& renderRequest, const RenderLayer& layer, const V3& position)
        : _render_request(renderRequest), _layer_snapshot(layer.snapshot()), _position(position) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _delegate->draw(graphicsContext);
    }

    virtual void run() override {
        _delegate = _layer_snapshot.render(_position);
        DASSERT(_delegate);
        _render_request.finish();
    }

private:
    RenderRequest _render_request;
    RenderLayer::Snapshot _layer_snapshot;
    V3 _position;

    sp<RenderCommand> _delegate;
};

}

RenderRequest::RenderRequest(const sp<Executor>& executor, const sp<OCSQueue<sp<RenderCommand>>>& renderCommands)
    : _stub(sp<Stub>::make(executor, renderCommands))
{
}

RenderRequest::RenderRequest(const sp<RenderRequest::Stub>& stub)
    : _stub(stub)
{
}

void RenderRequest::finish()
{
    _stub->onJobDone();
    _stub = nullptr;
}

void RenderRequest::addRequest(const sp<RenderCommand>& renderCommand)
{
    _stub->_render_command_pipe_line->add(renderCommand);
}

void RenderRequest::addBackgroundRequest(const RenderLayer& layer, const V3& position)
{
    const sp<BackgroundRenderCommand> renderCommand = sp<BackgroundRenderCommand>::make(*this, layer, position);
    ++(_stub->_background_renderer_count);
    _stub->_render_command_pipe_line->add(renderCommand);
    _stub->_executor->execute(renderCommand);
}

RenderRequest::Stub::Stub(const sp<Executor>& executor, const sp<OCSQueue<sp<RenderCommand>>>& renderCommands)
    : _executor(executor), _render_commands(renderCommands), _render_command_pipe_line(sp<RenderCommandPipeline>::make()), _background_renderer_count(1)
{
}

void RenderRequest::Stub::onJobDone()
{
    int32_t count = --_background_renderer_count;
    DCHECK(count >= 0, "Bad count: %d", count);
    if(count == 0)
    {
        DASSERT(_render_command_pipe_line);
        _render_commands->add(static_cast<sp<RenderCommand>>(_render_command_pipe_line));
        _render_command_pipe_line = nullptr;
    }
}

}
