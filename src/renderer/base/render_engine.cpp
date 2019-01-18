#include "renderer/base/render_engine.h"

#include "renderer/base/render_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderEngine::RenderEngine(Ark::RendererVersion version, const sp<RendererFactory>& rendererFactory)
    : _renderer_factory(rendererFactory), _render_context(_renderer_factory->initialize(version))
{
}

Ark::RendererVersion RenderEngine::version() const
{
    return _render_context->version();
}

const Viewport& RenderEngine::viewport() const
{
    return _render_context->viewport();
}

const sp<RendererFactory>& RenderEngine::rendererFactory() const
{
    return _renderer_factory;
}

const sp<RenderContext>& RenderEngine::renderContext() const
{
    return _render_context;
}

void RenderEngine::onSurfaceCreated()
{
    _renderer_factory->onSurfaceCreated(_render_context);
}

sp<RenderView> RenderEngine::createRenderView(const Viewport& viewport) const
{
    return _renderer_factory->createRenderView(_render_context, viewport);
}

}
