#include "graphics/base/render_layer.h"

#include "core/ark.h"
#include "core/base/named_hash.h"
#include "core/types/global.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_request.h"
#include "graphics/components/render_object.h"
#include "graphics/util/renderable_type.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/impl/model_loader/model_loader_cached.h"
#include "renderer/impl/model_loader/model_loader_ndc.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/drawing_context_composer.h"

namespace ark {

RenderLayer::Stub::Stub(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings, sp<Vec4> scissor)
    : _render_controller(std::move(renderController)), _model_loader(ModelLoaderCached::ensureCached(std::move(modelLoader))), _shader(std::move(shader)), _visible(std::move(visible), true),
      _discarded(std::move(discarded), false), _varyings(std::move(varyings)), _scissor(scissor ? std::move(scissor) : sp<Vec4>(_shader->pipelineDesciptor()->scissor())), _is_dynamic_scissor(false), _drawing_context_composer(_model_loader->makeRenderCommandComposer(_shader)),
      _pipeline_bindings(_drawing_context_composer->makePipelineBindings(_shader, _render_controller, _model_loader->renderMode())), _stride(_shader->layout()->getStreamLayout(0).stride())
{
    _model_loader->bind(_pipeline_bindings);
    const PipelineDescriptor::TraitScissorTest* scissorTest = _pipeline_bindings->pipelineDescriptor()->getTrait<PipelineDescriptor::TraitScissorTest>();
    CHECK(!_scissor || scissorTest, "RenderLayer has a scissor but its Shader has no scissor_test trait");
    if(_scissor)
        _is_dynamic_scissor = scissorTest->_dynamic;
}

RenderLayer::RenderLayer(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings, sp<Vec4> scissor)
    : RenderLayer(sp<Stub>::make(std::move(renderController), std::move(modelLoader), std::move(shader), std::move(visible), std::move(discarded), std::move(varyings), std::move(scissor)))
{
}

RenderLayer::RenderLayer(sp<RenderLayer::Stub> stub)
    : _stub(std::move(stub))
{
}

RenderLayerSnapshot RenderLayer::snapshot(const RenderRequest& renderRequest)
{
    DPROFILER_TRACE("Snapshot");

    RenderLayerSnapshot renderLayerSnapshot(renderRequest, _stub);
    renderLayerSnapshot.addLayerContext(renderRequest, _layer_contexts);
    renderLayerSnapshot.snapshot(renderRequest);

    DPROFILER_LOG("Signature", _stub->_pipeline_bindings->pipelineDescriptor()->signature());
    DPROFILER_LOG("verticesDirty", renderLayerSnapshot.verticesDirty());
    return renderLayerSnapshot;
}

const sp<ModelLoader>& RenderLayer::modelLoader() const
{
    return _stub->_model_loader;
}

sp<Boolean> RenderLayer::visible() const
{
    return _stub->_visible.toVar();
}

void RenderLayer::setVisible(sp<Boolean> visible) const
{
    _stub->_visible.reset(std::move(visible));
}

const sp<Shader>& RenderLayer::shader() const
{
    return _stub->_shader;
}

sp<LayerContext> RenderLayer::makeLayerContext(sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded, sp<Updatable> updatable) const
{
    return sp<LayerContext>::make(_stub->_shader, modelLoader ? sp<ModelLoader>::make<ModelLoaderCached>(std::move(modelLoader)) : _stub->_model_loader, std::move(position), std::move(visible), std::move(discarded), _stub->_varyings, std::move(updatable));
}

sp<LayerContext> RenderLayer::addLayerContext(sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded)
{
    sp<LayerContext> layerContext = makeLayerContext(std::move(modelLoader), std::move(position), std::move(visible), std::move(discarded));
    _layer_contexts.push_back(layerContext);
    return layerContext;
}

void RenderLayer::addLayerContext(sp<LayerContext> layerContext)
{
    _layer_contexts.push_back(std::move(layerContext));
}

sp<Layer> RenderLayer::makeLayer(sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded)
{
    return sp<Layer>::make(addLayerContext(std::move(modelLoader), std::move(position), std::move(visible), std::move(discarded)));
}

void RenderLayer::render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator)
{
    renderRequest.addRenderCommand(compose(renderRequest, drawDecorator));
}

sp<RenderCommand> RenderLayer::compose(const RenderRequest& renderRequest, sp<DrawDecorator> drawDecorator)
{
    return snapshot(renderRequest).compose(renderRequest, std::move(drawDecorator));
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : BUILDER(factory, manifest, nullptr, nullptr)
{
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, sp<Builder<ModelLoader>> modelLoader, sp<Builder<Shader>> shader)
    : _model_loader(modelLoader ? std::move(modelLoader) : factory.ensureBuilder<ModelLoader>(manifest, constants::MODEL_LOADER)), _shader(shader ? std::move(shader) : factory.ensureBuilder<Shader>(manifest, constants::SHADER)),
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
    : _impl(factory, manifest, sp<Builder<ModelLoader>>::make<Builder<ModelLoader>::Prebuilt>(sp<ModelLoader>::make<ModelLoaderNDC>()), Shader::makeBuilder(factory, manifest, "shaders/ndc.vert", ""))
{
}

sp<Renderer> RenderLayer::RENDERER_POST_PROCESS::build(const Scope& args)
{
    sp<RenderLayer> renderLayer = _impl.build(args);
    sp<LayerContext> layerContext = sp<LayerContext>::make(nullptr, sp<ModelLoader>::make<ModelLoaderNDC>(), nullptr, nullptr, Global<Constants>()->BOOLEAN_FALSE);
    sp<Renderable> renderable = RenderableType::create(sp<Renderable>::make<RenderObject>(NamedHash(1), sp<Vec3>::make<Vec3::Const>(V3(0, 0, 1.0f))), Global<Constants>()->BOOLEAN_FALSE);
    layerContext->pushBack(std::move(renderable));
    renderLayer->addLayerContext(std::move(layerContext));
    return renderLayer;
}

}
