#include "renderer/base/render_engine.h"

#include "renderer/base/gl_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderEngine::RenderEngine(Ark::GLVersion version, const sp<RendererFactory>& rendererFactory)
    : _renderer_factory(rendererFactory), _gl_context(sp<GLContext>::make(version))
{
    if(version != Ark::AUTO)
        _renderer_factory->setGLVersion(version, _gl_context);
}

const sp<RendererFactory>& RenderEngine::rendererFactory() const
{
    return _renderer_factory;
}

void RenderEngine::initialize()
{
    _renderer_factory->initialize(_gl_context);
}

sp<RenderView> RenderEngine::createRenderView(const Viewport& viewport) const
{
    return _renderer_factory->createRenderView(_gl_context, viewport);
}

}
