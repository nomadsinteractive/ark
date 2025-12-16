#include "renderer/base/render_engine.h"

#include "core/types/global.h"

#include "graphics/base/camera.h"

#include "renderer/base/render_engine_context.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderEngine::RenderEngine(const ApplicationManifest::Renderer& renderer, sp<RendererFactory> rendererFactory)
    : _render_context(rendererFactory->createRenderEngineContext(renderer)), _renderer_factory(std::move(rendererFactory)), _coordinate_system(renderer._coordinate_system == enums::COORDINATE_SYSTEM_DEFAULT ? _render_context->viewportCoordinateSystem() : renderer._coordinate_system)
{
}

enums::RendererVersion RenderEngine::version() const
{
    return _render_context->version();
}

enums::CoordinateSystem RenderEngine::viewportCoordinateSystem() const
{
    return _render_context->viewportCoordinateSystem();
}

enums::CoordinateSystem RenderEngine::ndcCoordinateSystem() const
{
    return _render_context->ndcCoordinateSystem();
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

float RenderEngine::toLayoutDirection(const float direction) const
{
    return _coordinate_system == enums::COORDINATE_SYSTEM_RHS ? -direction : direction;
}

bool RenderEngine::isLHS() const
{
    return _coordinate_system == enums::COORDINATE_SYSTEM_LHS;
}

bool RenderEngine::isBackendLHS() const
{
    return _render_context->viewportCoordinateSystem() == enums::COORDINATE_SYSTEM_LHS;
}

bool RenderEngine::isViewportFlipped() const
{
    return _coordinate_system != _render_context->viewportCoordinateSystem();
}

bool RenderEngine::shouldFlipY() const
{
    return isBackendLHS() && ndcCoordinateSystem() != viewportCoordinateSystem();
}

V2 RenderEngine::toLHSPosition(const V2& position) const
{
    if(_coordinate_system == enums::COORDINATE_SYSTEM_RHS)
        return {position.x(), _render_context->viewport().height() - position.y()};
    return position;
}

Rect RenderEngine::toRendererRect(const Rect& scissor, const enums::CoordinateSystem cs) const
{
    Rect s(scissor);
    s.scale(_render_context->displayUnit());
    if(viewportCoordinateSystem() != (cs == enums::COORDINATE_SYSTEM_DEFAULT ? _coordinate_system : cs))
        s.vflip(static_cast<float>(_render_context->displayResolution().height));
    return s;
}

V2 RenderEngine::toNDC(const float viewportX, const float viewportY) const
{
    const Viewport& viewport = _render_context->viewport();
    const float ndcx = viewportX * 2.0f / viewport.width() - 1.0f;
    const float ndcy = viewportY * 2.0f / viewport.height() - 1.0f;
    return {ndcx, isViewportFlipped() ? -ndcy : ndcy};
}

void RenderEngine::onSurfaceCreated()
{
    _renderer_factory->onSurfaceCreated(*this);
}

sp<RenderView> RenderEngine::createRenderView(const sp<RenderController>& renderController, const Viewport& viewport) const
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

sp<Pipeline> RenderEngine::createPipeline(GraphicsContext& graphicsContext, const PipelineBindings& pipelineBindings, std::map<enums::ShaderStageBit, String> stages)
{
    if(!_pipeline_factory)
        _pipeline_factory = _renderer_factory->createPipelineFactory();
    return _pipeline_factory->buildPipeline(graphicsContext, pipelineBindings, std::move(stages));
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
