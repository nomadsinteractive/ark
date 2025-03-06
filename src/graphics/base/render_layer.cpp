#include "graphics/base/render_layer.h"

#include "core/base/observer.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_request.h"
#include "graphics/impl/render_batch/render_batch_post_process.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/shader_layout.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/impl/model_loader/model_loader_cached.h"
#include "renderer/impl/model_loader/model_loader_ndc.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/drawing_context_composer.h"

namespace ark {

class RenderLayer::RenderBatchImpl final : public RenderBatch {
public:

    Vector<sp<LayerContext>>& snapshot(const RenderRequest& renderRequest) override
    {
        return _layer_contexts;
    }

    void addLayerContext(sp<LayerContext> layerContext)
    {
        _layer_contexts.push_back(std::move(layerContext));
    }

private:
    Vector<sp<LayerContext>> _layer_contexts;
};

RenderLayer::Stub::Stub(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings, sp<Vec4> scissor)
    : _render_controller(std::move(renderController)), _model_loader(ModelLoaderCached::ensureCached(std::move(modelLoader))), _shader(std::move(shader)), _visible(std::move(visible), true),
      _discarded(std::move(discarded), false), _varyings(std::move(varyings)), _scissor(std::move(scissor)), _drawing_context_composer(_model_loader->makeRenderCommandComposer(_shader)),
      _pipeline_bindings(_drawing_context_composer->makePipelineBindings(_shader, _render_controller, _model_loader->renderMode())), _stride(_shader->layout()->getStreamLayout(0).stride())
{
    _model_loader->bind(_pipeline_bindings);
    CHECK(!_scissor || _pipeline_bindings->pipelineDescriptor()->hasTrait(PipelineDescriptor::TRAIT_TYPE_SCISSOR_TEST), "RenderLayer has a scissor while its Shader has no scissor_test trait");
}

RenderLayer::RenderLayer(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings, sp<Vec4> scissor)
    : RenderLayer(sp<Stub>::make(std::move(renderController), std::move(modelLoader), std::move(shader), std::move(visible), std::move(discarded), std::move(varyings), std::move(scissor)))
{
}

RenderLayer::RenderLayer(sp<RenderLayer::Stub> stub)
    : _stub(std::move(stub)), _render_batch(sp<RenderBatchImpl>::make()), _render_batches{_render_batch}
{
}

RenderLayerSnapshot RenderLayer::snapshot(const RenderRequest& renderRequest)
{
    DPROFILER_TRACE("Snapshot");

    RenderLayerSnapshot renderLayerSnapshot(renderRequest, _stub);
    for(auto iter = _render_batches.begin(); iter != _render_batches.end();)
    {
        const sp<RenderBatch>& i = *iter;
        Vector<sp<LayerContext>>& layerContexts = i->snapshot(renderRequest);
        if(i->discarded() ? i->discarded()->val() : i.unique())
        {
            renderLayerSnapshot.addDiscardedLayerContexts(layerContexts);
            iter = _render_batches.erase(iter);
        }
        else
        {
            renderLayerSnapshot.addLayerContext(renderRequest, layerContexts);
            ++iter;
        }
    }

    renderLayerSnapshot.snapshot(renderRequest);

    DPROFILER_LOG("NeedsReload", renderLayerSnapshot.needsReload());
    return renderLayerSnapshot;
}

const sp<ModelLoader>& RenderLayer::modelLoader() const
{
    return _stub->_model_loader;
}

sp<LayerContext> RenderLayer::makeLayerContext(sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded) const
{
    return sp<LayerContext>::make(_stub->_shader, modelLoader ? sp<ModelLoader>::make<ModelLoaderCached>(std::move(modelLoader)) : _stub->_model_loader, std::move(position), std::move(visible), std::move(discarded), _stub->_varyings);
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
        layerContext->setVaryings(_stub->_varyings);
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
    renderRequest.addRenderCommand(compose(renderRequest));
}

sp<RenderCommand> RenderLayer::compose(const RenderRequest& renderRequest)
{
    return snapshot(renderRequest).compose(renderRequest);
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : BUILDER(factory, manifest, nullptr)
{
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, sp<Builder<ModelLoader>> modelLoader)
    : _model_loader(modelLoader ? std::move(modelLoader) : factory.ensureBuilder<ModelLoader>(manifest, constants::MODEL_LOADER)), _shader(factory.ensureBuilder<Shader>(manifest, constants::SHADER)),
      _varyings(factory.getConcreteClassBuilder<Varyings>(manifest, constants::VARYINGS)), _visible(factory.getBuilder<Boolean>(manifest, constants::VISIBLE)), _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED)),
      _scissor(factory.getBuilder<Vec4>(manifest, "scissor"))
{
}

sp<RenderLayer> RenderLayer::BUILDER::build(const Scope& args)
{
    return sp<RenderLayer>::make(Ark::instance().renderController(), _model_loader->build(args), _shader->build(args), _visible.build(args), _discarded.build(args), _varyings->build(args), _scissor.build(args));
}

RenderLayer::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest)
    : _impl(factory, manifest)
{
}

sp<Renderer> RenderLayer::RENDERER_BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

RenderLayer::RENDERER_POST_PROCESS::RENDERER_POST_PROCESS(BeanFactory& factory, const document& manifest)
    : _impl(factory, manifest, sp<Builder<ModelLoader>>::make<Builder<ModelLoader>::Prebuilt>(sp<ModelLoader>::make<ModelLoaderNDC>()))
{
}

sp<Renderer> RenderLayer::RENDERER_POST_PROCESS::build(const Scope& args)
{
    sp<RenderLayer> renderLayer = _impl.build(args);
    renderLayer->addRenderBatch(sp<RenderBatch>::make<RenderBatchPostProcess>());
    return renderLayer;
}

}
