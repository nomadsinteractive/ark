#include "graphics/base/render_layer.h"

#include "core/base/observer.h"
#include "core/base/notifier.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/base/v4.h"
#include "graphics/impl/render_batch/render_batch_impl.h"
#include "graphics/impl/render_batch/render_batch_with_translation.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/impl/model_loader/model_loader_cached.h"
#include "renderer/inf/animation.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

RenderLayer::Stub::Stub(sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Vec4> scissor, sp<RenderController> renderController)
    : _model_loader(ModelLoaderCached::ensureCached(std::move(modelLoader))), _shader(std::move(shader)), _scissor(std::move(scissor)), _render_controller(std::move(renderController)), _render_command_composer(_model_loader->makeRenderCommandComposer()),
      _shader_bindings(_render_command_composer->makeShaderBindings(_shader, _render_controller, _model_loader->renderMode())), _visible(nullptr, true), _layer(sp<Layer>::make(sp<LayerContext>::make(sp<RenderBatchImpl>::make(), _model_loader))),
      _stride(_shader->input()->getStream(0).stride())
{
    _model_loader->initialize(_shader_bindings);
    DCHECK(!_scissor || _shader_bindings->pipelineBindings()->hasFlag(PipelineBindings::FLAG_DYNAMIC_SCISSOR, PipelineBindings::FLAG_DYNAMIC_SCISSOR_BITMASK), "RenderLayer has a scissor while its Shader has no FLAG_DYNAMIC_SCISSOR set");
}

sp<LayerContext> RenderLayer::Stub::makeLayerContext(sp<RenderBatch> batch, sp<ModelLoader> modelLoader, sp<Boolean> visible, sp<Boolean> disposed)
{
    sp<LayerContext> layerContext = sp<LayerContext>::make(std::move(batch), modelLoader ? sp<ModelLoader>::make<ModelLoaderCached>(std::move(modelLoader)) : _model_loader, std::move(visible), std::move(disposed), _layer->context()->varyings());
    _batch_groups.push_back(layerContext);
    return layerContext;
}

void RenderLayer::Stub::addLayerContext(sp<LayerContext> layerContext)
{
    if(!layerContext->modelLoader())
        layerContext->setModelLoader(_model_loader);
    if(!layerContext->varyings())
        layerContext->setVaryings(_layer->context()->varyings());
    _batch_groups.push_back(std::move(layerContext));
}

RenderLayer::RenderLayer(sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Vec4> scissor, sp<RenderController> renderController)
    : RenderLayer(sp<Stub>::make(std::move(modelLoader), std::move(shader), std::move(scissor), std::move(renderController)))
{
}

RenderLayer::RenderLayer(const sp<RenderLayer::Stub>& stub)
    : _stub(stub)
{
}

const sp<LayerContext>& RenderLayer::context() const
{
    return _stub->_layer->context();
}

RenderLayerSnapshot RenderLayer::snapshot(RenderRequest& renderRequest) const
{
    return RenderLayerSnapshot(renderRequest, _stub);
}

const sp<Layer>& RenderLayer::layer() const
{
    return _stub->_layer;
}

sp<LayerContext> RenderLayer::makeLayerContext(sp<RenderBatch> batchOptional, sp<ModelLoader> modelLoader, sp<Boolean> visible, sp<Boolean> disposed, sp<Vec3> position) const
{
    sp<RenderBatch> renderBatch = batchOptional ? std::move(batchOptional) : sp<RenderBatch>::make<RenderBatchImpl>();
    if(position)
        renderBatch = sp<RenderBatchWithTranslation>::make(std::move(renderBatch), std::move(position));
    return _stub->makeLayerContext(std::move(renderBatch), std::move(modelLoader), std::move(visible), std::move(disposed));
}

void RenderLayer::addLayerContext(sp<LayerContext> layerContext)
{
    _stub->addLayerContext(std::move(layerContext));
}

sp<Layer> RenderLayer::makeLayer(sp<ModelLoader> modelLoader, sp<Boolean> visible, sp<Boolean> disposed, sp<Vec3> position) const
{
    return sp<Layer>::make(makeLayerContext(nullptr, std::move(modelLoader), std::move(visible), std::move(disposed), std::move(position)));
}

void RenderLayer::render(RenderRequest& renderRequest, const V3& position)
{
    _stub->_layer->render(renderRequest, position);
    renderRequest.addBackgroundRequest(*this, position);
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : BUILDER(factory, manifest, resourceLoaderContext, factory.ensureBuilder<ModelLoader>(manifest, Constants::Attributes::MODEL))
{
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, sp<Builder<ModelLoader>> modelLoader, sp<Builder<Shader>> shader)
    : _resource_loader_context(resourceLoaderContext), _layers(factory.makeBuilderList<Layer>(manifest, Constants::Attributes::LAYER)), _model_loader(std::move(modelLoader)), _shader(shader ? std::move(shader) : Shader::fromDocument(factory, manifest, resourceLoaderContext)),
      _scissor(factory.getBuilder<Vec4>(manifest, "scissor"))
{
}

sp<RenderLayer> RenderLayer::BUILDER::build(const Scope& args)
{
    const sp<RenderLayer> renderLayer = sp<RenderLayer>::make(_model_loader->build(args), _shader->build(args), _scissor->build(args), _resource_loader_context->renderController());
    for(const sp<Builder<Layer>>& i : _layers)
    {
        const sp<Layer> layer = i->build(args);
        renderLayer->addLayerContext(layer->context());
    }
    return renderLayer;
}

RenderLayer::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext)
{
}

sp<Renderer> RenderLayer::RENDERER_BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
