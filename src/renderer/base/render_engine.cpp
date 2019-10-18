#include "renderer/base/render_engine.h"

#include "core/types/global.h"

#include "graphics/base/camera.h"

#include "renderer/base/render_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderEngine::RenderEngine(Ark::RendererVersion version, Ark::RendererCoordinateSystem coordinateSystem, const sp<RendererFactory>& rendererFactory)
    : _coordinate_system(coordinateSystem), _renderer_factory(rendererFactory), _render_context(_renderer_factory->initialize(version))
{
}

Ark::RendererVersion RenderEngine::version() const
{
    return _render_context->version();
}

const sp<RendererFactory>& RenderEngine::rendererFactory() const
{
    return _renderer_factory;
}

const sp<RenderContext>& RenderEngine::renderContext() const
{
    return _render_context;
}

const Viewport& RenderEngine::viewport() const
{
    return _render_context->viewport();
}

float RenderEngine::toLayoutDirection(float direction) const
{
    return _coordinate_system == Ark::COORDINATE_SYSTEM_RHS ? -direction : direction;
}

Rect RenderEngine::toRendererScissor(const Rect& scissor, Ark::RendererCoordinateSystem cs) const
{
    Rect s(scissor);
    s.scale(_render_context->displayUnit());
    if(_render_context->coordinateSystem() != (cs == Ark::COORDINATE_SYSTEM_DEFAULT ? _coordinate_system : cs))
        s.vflip(static_cast<float>(_render_context->displayResolution().height));
    return s;
}

void RenderEngine::onSurfaceCreated()
{
    _renderer_factory->onSurfaceCreated(_render_context);
}

sp<RenderView> RenderEngine::createRenderView(const sp<RenderController>& renderController, const Viewport& viewport) const
{
    const Global<Camera> mainCamera;
    mainCamera->ortho(viewport.left(), viewport.right(), viewport.top(), viewport.bottom(), viewport.near(), viewport.far(), _render_context->coordinateSystem());

    _render_context->setViewport(viewport);
    return _renderer_factory->createRenderView(_render_context, renderController);
}

}
