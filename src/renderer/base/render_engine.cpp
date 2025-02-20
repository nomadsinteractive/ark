#include "renderer/base/render_engine.h"

#include "core/types/global.h"

#include "graphics/base/camera.h"

#include "renderer/base/render_engine_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderEngine::RenderEngine(const ApplicationManifest::Renderer& renderer, sp<RendererFactory> rendererFactory)
    : _coordinate_system(renderer._coordinate_system == Ark::COORDINATE_SYSTEM_DEFAULT ? rendererFactory->features()._default_coordinate_system : renderer._coordinate_system), _renderer_factory(std::move(rendererFactory)),
      _render_context(_renderer_factory->createRenderEngineContext(renderer))
{
}

Enum::RendererVersion RenderEngine::version() const
{
    return _render_context->version();
}

Ark::RendererCoordinateSystem RenderEngine::coordinateSystem() const
{
    return _renderer_factory->features()._default_coordinate_system;
}

const sp<RendererFactory>& RenderEngine::rendererFactory() const
{
    return _renderer_factory;
}

const sp<RenderEngineContext>& RenderEngine::context() const
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

bool RenderEngine::isLHS() const
{
    return _coordinate_system == Ark::COORDINATE_SYSTEM_LHS;
}

bool RenderEngine::isBackendLHS() const
{
    return _renderer_factory->features()._default_coordinate_system == Ark::COORDINATE_SYSTEM_LHS;
}

bool RenderEngine::isYUp() const
{
    return coordinateSystem() == Ark::COORDINATE_SYSTEM_RHS;
}

bool RenderEngine::isViewportFlipped() const
{
    return _coordinate_system != _renderer_factory->features()._default_coordinate_system;
}

V2 RenderEngine::toLHSPosition(const V2& position) const
{
    if(_coordinate_system == Ark::COORDINATE_SYSTEM_RHS)
        return {position.x(), _render_context->viewport().height() - position.y()};
    return position;
}

Rect RenderEngine::toRendererRect(const Rect& scissor, Ark::RendererCoordinateSystem cs) const
{
    Rect s(scissor);
    s.scale(_render_context->displayUnit());
    if(coordinateSystem() != (cs == Ark::COORDINATE_SYSTEM_DEFAULT ? _coordinate_system : cs))
        s.vflip(static_cast<float>(_render_context->displayResolution().height));
    return s;
}

V2 RenderEngine::toNDC(float viewportX, float viewportY) const
{
    const Viewport& viewport = _render_context->viewport();
    const float ndcx = (viewportX * 2 - viewport.width()) / viewport.width();
    const float ndcy = (viewportY * 2 - viewport.height()) / viewport.height();
    return {ndcx, isViewportFlipped() ? -ndcy : ndcy};
}

void RenderEngine::onSurfaceCreated()
{
    _renderer_factory->onSurfaceCreated(*this);
}

sp<RenderView> RenderEngine::createRenderView(const sp<RenderController>& renderController, const Viewport& viewport) const
{
    const Global<Camera> mainCamera;
    float clipNear = viewport.clipNear();
    float clipFar = viewport.clipFar();
    if(isLHS())
        std::swap(clipNear, clipFar);
    if(isBackendLHS())
        mainCamera->ortho(viewport.left(), viewport.right(), viewport.top(), viewport.bottom(), clipNear, clipFar);
    else
        mainCamera->ortho(viewport.left(), viewport.right(), viewport.bottom(), viewport.top(), clipNear, clipFar);

    _render_context->setViewport(viewport);
    return _renderer_factory->createRenderView(_render_context, renderController);
}

const RenderEngine::PlatformInfo& RenderEngine::info() const
{
    return _info;
}

RenderEngine::PlatformInfo& RenderEngine::info()
{
    return _info;
}

}
