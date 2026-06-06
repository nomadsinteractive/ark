#include "renderer/base/render_backend.h"

#include "core/types/global.h"

#include "graphics/base/camera.h"

#include "renderer/base/render_backend_info.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderBackend::RenderBackend(const ApplicationManifest::Renderer& renderer, sp<RendererFactory> rendererFactory)
    : _render_context(rendererFactory->createRenderBackendInfo(renderer)), _renderer_factory(std::move(rendererFactory)), _coordinate_system(renderer._coordinate_system == enums::COORDINATE_SYSTEM_DEFAULT ? _render_context->viewportCoordinateSystem() : renderer._coordinate_system)
{
}

enums::RendererVersion RenderBackend::version() const
{
    return _render_context->version();
}

enums::CoordinateSystem RenderBackend::viewportCoordinateSystem() const
{
    return _render_context->viewportCoordinateSystem();
}

enums::CoordinateSystem RenderBackend::ndcCoordinateSystem() const
{
    return _render_context->ndcCoordinateSystem();
}

const sp<RendererFactory>& RenderBackend::rendererFactory() const
{
    return _renderer_factory;
}

const sp<RenderBackendInfo>& RenderBackend::info() const
{
    return _render_context;
}

const Viewport& RenderBackend::viewport() const
{
    return _render_context->viewport();
}

float RenderBackend::toLayoutDirection(const float direction) const
{
    return _coordinate_system == enums::COORDINATE_SYSTEM_RHS ? -direction : direction;
}

bool RenderBackend::isLHS() const
{
    return _coordinate_system == enums::COORDINATE_SYSTEM_LHS;
}

bool RenderBackend::isBackendLHS() const
{
    return _render_context->viewportCoordinateSystem() == enums::COORDINATE_SYSTEM_LHS;
}

bool RenderBackend::isViewportFlipped() const
{
    return _coordinate_system != _render_context->viewportCoordinateSystem();
}

bool RenderBackend::shouldFlipY() const
{
    return isBackendLHS() && ndcCoordinateSystem() != viewportCoordinateSystem();
}

V2 RenderBackend::toLHSPosition(const V2& position) const
{
    if(_coordinate_system == enums::COORDINATE_SYSTEM_RHS)
        return {position.x(), _render_context->viewport().height() - position.y()};
    return position;
}

Rect RenderBackend::toRendererRect(const Rect& scissor, const enums::CoordinateSystem cs) const
{
    Rect s(scissor);
    s.scale(_render_context->displayUnit());
    if(viewportCoordinateSystem() != (cs == enums::COORDINATE_SYSTEM_DEFAULT ? _coordinate_system : cs))
        s.vflip(static_cast<float>(_render_context->displayResolution().height));
    return s;
}

V2 RenderBackend::toNDC(const float viewportX, const float viewportY) const
{
    const Viewport& viewport = _render_context->viewport();
    const float ndcx = viewportX * 2.0f / viewport.width() - 1.0f;
    const float ndcy = viewportY * 2.0f / viewport.height() - 1.0f;
    return {ndcx, isViewportFlipped() ? -ndcy : ndcy};
}

void RenderBackend::onSurfaceCreated()
{
    _renderer_factory->onSurfaceCreated(*this);
}

sp<RenderView> RenderBackend::createRenderView(const sp<RenderController>& renderController, const Viewport& viewport) const
{
    const Global<Camera> mainCamera;
    const float clipNear = viewport.clipNear();
    const float clipFar = viewport.clipFar();
    if(isLHS())
        mainCamera->ortho(viewport.left(), viewport.right(), viewport.top(), viewport.bottom(), clipFar, clipNear);
    else
        mainCamera->ortho(viewport.left(), viewport.right(), viewport.bottom(), viewport.top(), clipNear, clipFar);

    _render_context->setViewport(viewport);
    return _renderer_factory->createRenderView(_render_context, renderController);
}

sp<Pipeline> RenderBackend::createPipeline(GraphicsContext& graphicsContext, const PipelineBindings& pipelineBindings, std::map<enums::ShaderStageBit, String> stages)
{
    if(!_pipeline_factory)
        _pipeline_factory = _renderer_factory->createPipelineFactory();
    return _pipeline_factory->buildPipeline(graphicsContext, pipelineBindings, std::move(stages));
}

const RenderBackend::PlatformInfo& RenderBackend::platformInfo() const
{
    return _platform_info;
}

RenderBackend::PlatformInfo& RenderBackend::platformInfo()
{
    return _platform_info;
}

}
