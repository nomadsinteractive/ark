#include "renderer/base/render_engine.h"

#include "core/types/global.h"

#include "graphics/base/camera.h"
#include "graphics/base/mat.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/render_engine_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderEngine::RenderEngine(Ark::RendererVersion version, Ark::RendererCoordinateSystem coordinateSystem, sp<RendererFactory> rendererFactory)
    : _coordinate_system(coordinateSystem == Ark::COORDINATE_SYSTEM_DEFAULT ? rendererFactory->defaultCoordinateSystem() : coordinateSystem), _renderer_factory(std::move(rendererFactory)),
      _render_context(_renderer_factory->initialize(version))
{
}

Ark::RendererVersion RenderEngine::version() const
{
    return _render_context->version();
}

Ark::RendererCoordinateSystem RenderEngine::coordinateSystem() const
{
    return _renderer_factory->defaultCoordinateSystem();
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

bool RenderEngine::isRendererLHS() const
{
    return coordinateSystem() == Ark::COORDINATE_SYSTEM_LHS;
}

bool RenderEngine::isViewportFlipped() const
{
    return _coordinate_system != _renderer_factory->defaultCoordinateSystem();
}

V2 RenderEngine::toViewportPosition(const V2& position) const
{
    if(isViewportFlipped())
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

V3 RenderEngine::toWorldPosition(const M4& vpMatrix, float screenX, float screenY, float z) const
{
    const Viewport& viewport = _render_context->viewport();

    const float ndcx = (screenX * 2 - viewport.width()) / viewport.width();
    const float ndcy = (screenY * 2 - viewport.height()) / viewport.height();
    const M4 vpInverse = MatrixUtil::inverse(vpMatrix);
    const V4 pos = MatrixUtil::mul(vpInverse, V4(ndcx, isViewportFlipped() ? -ndcy : ndcy, z, 1.0f));
    return {pos.x() / pos.w(), pos.y() / pos.w(), pos.z() / pos.w()};
}

void RenderEngine::onSurfaceCreated()
{
    _renderer_factory->onSurfaceCreated(_render_context);
}

sp<RenderView> RenderEngine::createRenderView(const sp<RenderController>& renderController, const Viewport& viewport) const
{
    const Global<Camera> mainCamera;
    mainCamera->ortho(viewport.left(), viewport.right(), viewport.top(), viewport.bottom(), viewport.clipNear(), viewport.clipFar());

    _render_context->setViewport(viewport);
    return _renderer_factory->createRenderView(_render_context, renderController);
}

}
