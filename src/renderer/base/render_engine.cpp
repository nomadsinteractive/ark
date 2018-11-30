#include "renderer/base/render_engine.h"

#include "renderer/base/gl_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderEngine::RenderEngine(Ark::RendererVersion version, const sp<RendererFactory>& rendererFactory)
    : _renderer_factory(rendererFactory), _render_context(sp<GLContext>::make(version))
{
    if(version != Ark::AUTO)
        _renderer_factory->setGLVersion(version, _render_context);
}

Ark::RendererVersion RenderEngine::version() const
{
    return _render_context->version();
}

const sp<RendererFactory>& RenderEngine::rendererFactory() const
{
    return _renderer_factory;
}

void RenderEngine::initialize()
{
    _renderer_factory->initialize(_render_context);
}

sp<RenderView> RenderEngine::createRenderView(const Viewport& viewport) const
{
    return _renderer_factory->createRenderView(_render_context, viewport);
}

}
