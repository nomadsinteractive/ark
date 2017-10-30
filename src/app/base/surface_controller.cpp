#include "app/base/surface_controller.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/impl/renderer/renderer_group.h"
#include "graphics/inf/layer.h"

#include "app/base/application_context.h"

namespace ark {

SurfaceController::SurfaceController()
    : _renderers(sp<RendererGroup>::make()), _controls(sp<RendererGroup>::make()),
      _layers(sp<RendererGroup>::make()), _last_render_command(_render_command_pipe_line_pool.allocate<RenderCommandPipeline>()),
      _queue_length(0)
{
}

void SurfaceController::addRenderer(const sp<Renderer>& renderer)
{
    _renderers->addRenderer(renderer);
}

void SurfaceController::addControl(const sp<Renderer>& control)
{
    _controls->addRenderer(control);
}

void SurfaceController::addLayer(const sp<Layer>& layer)
{
    _layers->addRenderer(layer);
}

void SurfaceController::requestUpdate()
{
    if(_queue_length < 2)
    {
        const sp<RenderCommandPipeline> renderCommand = _render_command_pipe_line_pool.allocate<RenderCommandPipeline>();
        _renderers->render(renderCommand, 0, 0);
        _controls->render(renderCommand, 0, 0);
        _layers->render(renderCommand, 0, 0);
        _render_commands.push(renderCommand);
        _queue_length ++;
    }
}

sp<RenderCommandPipeline> SurfaceController::getRenderCommand()
{
    std::queue<sp<RenderCommandPipeline>>& synchronized = _render_commands.synchronized();
//    DWARN(!synchronized.empty(), "RenderCommandPipeLine queue is empty, frame skipped");
    if(!synchronized.empty())
    {
        _last_render_command = synchronized.front();
        synchronized.pop();
    }
    _queue_length = synchronized.size();
    return _last_render_command;
}

}
