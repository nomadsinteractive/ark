#include "graphics/base/render_layer.h"

#include "core/base/observer.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/base/v4.h"
#include "graphics/impl/render_batch/render_batch_impl.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/impl/model_loader/model_loader_cached.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

RenderLayer::Stub::Stub(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings, sp<Vec4> scissor)
    : _render_controller(std::move(renderController)), _model_loader(ModelLoaderCached::ensureCached(std::move(modelLoader))), _shader(std::move(shader)), _scissor(std::move(scissor)),
      _render_command_composer(_model_loader->makeRenderCommandComposer()), _pipeline_bindings(_render_command_composer->makeShaderBindings(_shader, _render_controller, _model_loader->renderMode())),
      _stride(_shader->input()->getStreamLayout(0).stride()), _layer_context(sp<LayerContext>::make(_shader, _model_loader, nullptr, std::move(visible), std::move(discarded), std::move(varyings)))
{
    _model_loader->initialize(_pipeline_bindings);
    CHECK(!_scissor || _pipeline_bindings->pipelineDescriptor()->hasFlag(PipelineDescriptor::FLAG_DYNAMIC_SCISSOR, PipelineDescriptor::FLAG_DYNAMIC_SCISSOR_BITMASK), "RenderLayer has a scissor while its Shader has no FLAG_DYNAMIC_SCISSOR set");
}

RenderLayer::RenderLayer(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings, sp<Vec4> scissor)
    : RenderLayer(sp<Stub>::make(std::move(renderController), std::move(modelLoader), std::move(shader), std::move(visible), std::move(discarded), std::move(varyings), std::move(scissor)))
{
}

RenderLayer::RenderLayer(sp<RenderLayer::Stub> stub)
    : _stub(std::move(stub)), _render_batch(sp<RenderBatchImpl>::make()), _render_batches{_render_batch}
{
}

const sp<LayerContext>& RenderLayer::context() const
{
    return _stub->_layer_context;
}

RenderLayerSnapshot RenderLayer::snapshot(RenderRequest& renderRequest)
{
    DPROFILER_TRACE("Snapshot");

    RenderLayerSnapshot renderLayerSnapshot(renderRequest, _stub);
    for(auto iter = _render_batches.begin(); iter != _render_batches.end();)
    {
        const sp<RenderBatch>& i = *iter;
        std::vector<sp<LayerContext>>& layerContexts = i->snapshot(renderRequest);
        if(i->discarded() ? i->discarded()->val() : i.unique())
        {
            renderLayerSnapshot.addDiscardedLayerContexts(layerContexts);
            iter = _render_batches.erase(iter);
        }
        else
        {
            renderLayerSnapshot.snapshot(renderRequest, layerContexts);
            ++iter;
        }
    }
    _stub->_render_command_composer->postSnapshot(_stub->_render_controller, renderLayerSnapshot);

    DPROFILER_LOG("NeedsReload", renderLayerSnapshot.needsReload());
    return renderLayerSnapshot;
}

sp<LayerContext> RenderLayer::makeLayerContext(sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded) const
{
    return sp<LayerContext>::make(_stub->_shader, modelLoader ? sp<ModelLoader>::make<ModelLoaderCached>(std::move(modelLoader)) : _stub->_model_loader, std::move(position), std::move(visible), std::move(discarded), _stub->_layer_context->varyings());
}

sp<LayerContext> RenderLayer::addLayerContext(sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded) const
{
    sp<LayerContext> layerContext = makeLayerContext(std::move(modelLoader), std::move(position), std::move(visible), std::move(discarded));
    _render_batch->addLayerContext(layerContext);
    return layerContext;
}

void RenderLayer::addLayerContext(sp<LayerContext> layerContext)
{
    if(!layerContext->modelLoader())
        layerContext->setModelLoader(_stub->_model_loader);
    if(!layerContext->varyings())
        layerContext->setVaryings(_stub->_layer_context->varyings());
    _render_batch->addLayerContext(std::move(layerContext));
}

void RenderLayer::addRenderBatch(sp<RenderBatch> renderBatch)
{
    _render_batches.push_back(std::move(renderBatch));
}

sp<Layer> RenderLayer::makeLayer(sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded) const
{
    return sp<Layer>::make(addLayerContext(std::move(modelLoader), std::move(position), std::move(visible), std::move(discarded)));
}

void RenderLayer::render(RenderRequest& renderRequest, const V3& position)
{
    renderRequest.addRenderCommand(snapshot(renderRequest).compose(renderRequest));
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : BUILDER(factory, manifest, resourceLoaderContext, factory.ensureBuilder<ModelLoader>(manifest, constants::MODEL_LOADER))
{
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, sp<Builder<ModelLoader>> modelLoader, sp<Builder<Shader>> shader)
    : _resource_loader_context(resourceLoaderContext), _layers(factory.makeBuilderList<Layer>(manifest, constants::LAYER)), _model_loader(std::move(modelLoader)), _shader(shader ? std::move(shader) : Shader::fromDocument(factory, manifest, resourceLoaderContext)),
      _varyings(factory.getConcreteClassBuilder<Varyings>(manifest, constants::VARYINGS)), _visible(factory.getBuilder<Boolean>(manifest, constants::VISIBLE)),
      _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED)), _scissor(factory.getBuilder<Vec4>(manifest, "scissor"))
{
}

sp<RenderLayer> RenderLayer::BUILDER::build(const Scope& args)
{
    const sp<RenderLayer> renderLayer = sp<RenderLayer>::make(_resource_loader_context->renderController(), _model_loader->build(args), _shader->build(args), _visible->build(args),
                                                              _discarded->build(args), _varyings->build(args), _scissor->build(args));
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
