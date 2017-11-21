#include "graphics/base/render_request.h"

#include "core/inf/executor.h"
#include "core/inf/runnable.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/surface_controller.h"
#include "graphics/inf/layer.h"

namespace ark {

namespace {

class BackgroundRenderCommand : public RenderCommand, public Runnable {
public:
    BackgroundRenderCommand(const sp<RenderRequest::Stub>& stub, const sp<Layer>& layer, float x, float y)
        : _stub(stub), _layer(layer), _layer_context_snapshot(layer->layerContext()->snapshot()), _x(x), _y(y) {
        layer->layerContext()->clear();
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        if(_delegate)
            _delegate->draw(graphicsContext);
    }

    virtual void run() override {
        _delegate = _layer->render(_layer_context_snapshot, _x, _y);
        _stub->onJobDone();
        _stub = nullptr;
    }

private:
    sp<RenderRequest::Stub> _stub;
    sp<Layer> _layer;
    LayerContext::Snapshot _layer_context_snapshot;
    float _x;
    float _y;

    sp<RenderCommand> _delegate;
};

}

RenderRequest::RenderRequest(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController)
    : _executor(executor), _stub(sp<Stub>::make(surfaceController))
{
}

void RenderRequest::start(const sp<RenderCommandPipeline>& renderCommandPipeline)
{
    DCHECK(!_stub->_render_command_pipe_line, "Illegal state, starting request on an unfinished RenderRequest");
    _stub->_render_command_pipe_line = renderCommandPipeline;
    _stub->_background_renderer_count.store(1);
}

void RenderRequest::finish()
{
    _stub->onJobDone();
}

bool RenderRequest::isFinished()
{
    return !_stub->_render_command_pipe_line;
}

void RenderRequest::addRequest(const sp<RenderCommand>& renderCommand)
{
    _stub->_render_command_pipe_line->add(renderCommand);
}

void RenderRequest::addBackgroundRequest(const sp<Layer>& layer, float x, float y)
{
    const sp<BackgroundRenderCommand> renderCommand = _render_command_pool.obtain<BackgroundRenderCommand>(_stub, layer, x, y);
    _stub->_background_renderer_count++;
    _stub->_render_command_pipe_line->add(renderCommand);
    _executor->execute(renderCommand);
}

RenderRequest::Stub::Stub(const sp<SurfaceController>& surfaceController)
    : _surface_controller(surfaceController)
{
}

void RenderRequest::Stub::onJobDone()
{
    int32_t count = --_background_renderer_count;
    DCHECK(count >= 0, "Bad count: %d", count);
    if(count == 0)
    {
        NOT_NULL(_render_command_pipe_line);
        _surface_controller->addRenderCommand(_render_command_pipe_line);
        _render_command_pipe_line = nullptr;
    }
}

}
