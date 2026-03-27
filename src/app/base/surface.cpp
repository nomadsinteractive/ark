#include "app/base/surface.h"

#include "core/ark.h"
#include "core/base/future.h"

#include "graphics/inf/render_view.h"
#include "graphics/base/surface_controller.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"

namespace ark {

namespace {

class SurfaceUpdateRequester final : public Runnable {
public:
    SurfaceUpdateRequester(sp<SurfaceController> surfaceController, const sp<ApplicationContext>& applicationContext)
        : _surface_controller(std::move(surfaceController)), _application_context(applicationContext)
    {
    }

    void run() override
    {
        DPROFILER_TRACE("SurfaceUpdate");
        DTHREAD_CHECK(THREAD_NAME_ID_CORE);
        const uint32_t tick = _application_context->onTick();
        _application_context->renderController()->onPreCompose(tick);
        _surface_controller->requestRender(tick);
    }

private:
    sp<SurfaceController> _surface_controller;
    sp<ApplicationContext> _application_context;
};

class RunnableScreenshot final : public Runnable {
public:
    RunnableScreenshot(sp<Bitmap> bitmap, sp<Future> future)
        : _bitmap(std::move(bitmap)), _future(std::move(future))
    {
    }

    void run() override
    {
        _future->notify(Box(_bitmap));
    }

private:
    sp<Bitmap> _bitmap;
    sp<Future> _future;
};

class RenderCommandScreenshot final : public RenderCommand {
public:
    RenderCommandScreenshot(ApplicationContext& applicationContext, RenderView& renderView, sp<RenderCommand> delegate, sp<Future> future)
        : _application_context(applicationContext), _render_view(renderView), _delegate(std::move(delegate)), _future(std::move(future))
    {
    }

    void draw(GraphicsContext& graphicsContext) override
    {
        _delegate->draw(graphicsContext);
        if(sp<Bitmap> bitmap = _render_view.doScreenshot())
            _application_context.runOnCoreThread(sp<RunnableScreenshot>::make(std::move(bitmap), std::move(_future)));
    }

private:
    ApplicationContext& _application_context;
    RenderView& _render_view;
    sp<RenderCommand> _delegate;
    sp<Future> _future;
};

}

Surface::Surface(sp<RenderView> renderView, const sp<ApplicationContext>& applicationContext)
    : _render_view(std::move(renderView)), _application_context(applicationContext), _surface_controller(sp<SurfaceController>::make()), _update_requester(sp<Runnable>::make<SurfaceUpdateRequester>(_surface_controller, _application_context))
{
    requestUpdate();
}

const sp<RenderView>& Surface::renderView() const
{
    return _render_view;
}

const sp<SurfaceController>& Surface::controller() const
{
    return _surface_controller;
}

void Surface::screenshot(sp<Future> future)
{
    _application_context->runOnMainThread([this, future=std::move(future)]() {
        onScreenshot(std::move(future));
    });
}

void Surface::requestUpdate() const
{
    _application_context->runOnCoreThread(_update_requester);
}

void Surface::onSurfaceCreated() const
{
    _render_view->onSurfaceCreated();
}

void Surface::onSurfaceChanged(const uint32_t width, const uint32_t height) const
{
    _render_view->onSurfaceChanged(width, height);
}

void Surface::onRenderFrame(const V4& backgroundColor)
{
    DTHREAD_CHECK(THREAD_NAME_ID_RENDERER);
    const RenderRequest renderRequest = _surface_controller->obtainRenderRequest();
    requestUpdate();
    DPROFILER_TRACE("onRenderFrame", ApplicationProfiler::CATEGORY_RENDERING);

    if(_screenshot_future)
    {
        RenderCommandScreenshot renderCommand(_application_context, _render_view, renderRequest.renderCommandPipeLine(), std::move(_screenshot_future));
        _render_view->onRenderFrame(backgroundColor, renderCommand);
    }
    else
        _render_view->onRenderFrame(backgroundColor, *renderRequest.renderCommandPipeLine());
}

void Surface::onScreenshot(sp<Future> future)
{
    DTHREAD_CHECK(THREAD_NAME_ID_RENDERER);
    _screenshot_future = std::move(future);
}

}
