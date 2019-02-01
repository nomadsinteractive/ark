#include "graphics/base/render_request.h"

#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/impl/runnable/runnable_with_callback.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/surface_controller.h"
#include "graphics/base/layer.h"

namespace ark {

namespace {

class BackgroundRenderCommand : public RenderCommand, public Runnable {
public:
    BackgroundRenderCommand(const Layer& layer, float x, float y)
        : _layer_snapshot(layer.snapshot()), _x(x), _y(y) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _delegate->draw(graphicsContext);
    }

    virtual void run() override {
        _delegate = _layer_snapshot.render(_x, _y);
        DASSERT(_delegate);
    }

private:
    Layer::Snapshot _layer_snapshot;
    float _x;
    float _y;

    sp<RenderCommand> _delegate;
};


class RenderCommandCallback : public Runnable {
public:
    RenderCommandCallback(const sp<RenderRequest::Stub>& stub)
        : _stub(stub) {
    }

    virtual void run() override {
        _stub->onJobDone(_stub);
    }

private:
    sp<RenderRequest::Stub> _stub;

};


}

RenderRequest::RenderRequest(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController, LFStack<RenderRequest>& renderRequestRecycler)
    : RenderRequest(sp<Stub>::make(executor, surfaceController, renderRequestRecycler))
{
}

RenderRequest::RenderRequest(const sp<RenderRequest::Stub>& stub)
    : _stub(stub), _callback(_stub->_object_pool.obtain<RenderCommandCallback>(_stub))
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
    _stub->onJobDone(_stub);
}

void RenderRequest::addRequest(const sp<RenderCommand>& renderCommand)
{
    _stub->_render_command_pipe_line->add(renderCommand);
}

void RenderRequest::addBackgroundRequest(const Layer& layer, float x, float y)
{
    const sp<BackgroundRenderCommand> renderCommand = _stub->_object_pool.obtain<BackgroundRenderCommand>(layer, x, y);
    ++(_stub->_background_renderer_count);
    _stub->_render_command_pipe_line->add(renderCommand);

    _stub->_executor->execute(_stub->_object_pool.obtain<RunnableWithCallback>(renderCommand, Observer(_callback, true)));
}

RenderRequest::Stub::Stub(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController, LFStack<RenderRequest>& renderRequestRecycler)
    : _executor(executor), _surface_controller(surfaceController), _render_request_recycler(renderRequestRecycler)
{
}

void RenderRequest::Stub::onJobDone(const sp<Stub>& self)
{
    int32_t count = --_background_renderer_count;
    DCHECK(count >= 0, "Bad count: %d", count);
    if(count == 0)
    {
        DASSERT(_render_command_pipe_line);
        _surface_controller->postRenderCommand(_render_command_pipe_line);
        _render_command_pipe_line = nullptr;
        _render_request_recycler.push(self);
    }
}

}
