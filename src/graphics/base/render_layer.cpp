#include "graphics/base/render_layer.h"

#include "core/base/observer.h"
#include "core/base/notifier.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/base/v4.h"

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
    : _model_loader(ModelLoaderCached::decorate(std::move(modelLoader))), _shader(std::move(shader)), _scissor(std::move(scissor)), _render_controller(std::move(renderController)), _render_command_composer(_model_loader->makeRenderCommandComposer()),
      _shader_bindings(_render_command_composer->makeShaderBindings(_shader, _render_controller, _model_loader->renderMode())), _visible(nullptr, true), _layer(sp<Layer>::make(sp<LayerContext>::make(nullptr, _model_loader, nullptr, nullptr))),
      _stride(_shader->input()->getStream(0).stride())
{
    _model_loader->initialize(_shader_bindings);
    DCHECK(!_scissor || _shader_bindings->pipelineBindings()->hasFlag(PipelineBindings::FLAG_DYNAMIC_SCISSOR, PipelineBindings::FLAG_DYNAMIC_SCISSOR_BITMASK), "RenderLayer has a scissor while its Shader has no FLAG_DYNAMIC_SCISSOR set");
}

sp<LayerContext> RenderLayer::Stub::makeLayerContext(sp<Batch> batch, sp<ModelLoader> modelLoader, sp<Boolean> visible)
{
    sp<LayerContext> layerContext = sp<LayerContext>::make(std::move(batch), modelLoader ? sp<ModelLoaderCached>::make(std::move(modelLoader)) : _model_loader, std::move(visible), _layer->context()->varyings());
    _batch_groups.push_back(layerContext);
    return layerContext;
}

RenderLayer::Snapshot::Snapshot(RenderRequest& renderRequest, const sp<Stub>& stub)
    : _stub(stub), _index_count(0)
{
    DPROFILER_TRACE("Snapshot");

    bool needsReload = _stub->_layer->context()->preSnapshot(renderRequest);

    for(auto iter = _stub->_batch_groups.begin(); iter != _stub->_batch_groups.end(); )
    {
        const sp<LayerContext>& layerContext = *iter;
        if(layerContext.unique())
        {
            iter = _stub->_batch_groups.erase(iter);
            needsReload = true;
        }
        else
        {
            needsReload = layerContext->preSnapshot(renderRequest) || needsReload;
            ++iter;
        }
    }

    _flag = needsReload ? SNAPSHOT_FLAG_RELOAD : SNAPSHOT_FLAG_DYNAMIC_UPDATE;

    _stub->_layer->context()->snapshot(renderRequest, *this);
    for(LayerContext& i : stub->_batch_groups)
        i.snapshot(renderRequest, *this);

    _stub->_render_command_composer->postSnapshot(_stub->_render_controller, *this);

    _ubos = _stub->_shader->takeUBOSnapshot(renderRequest);
    _ssbos = _stub->_shader->takeSSBOSnapshot(renderRequest);

    if(_stub->_scissor && _stub->_scissor->update(renderRequest.timestamp()))
        _scissor = Rect(_stub->_scissor->val());

    DPROFILER_LOG("NeedsReload", needsReload);
}

sp<RenderCommand> RenderLayer::Snapshot::render(const RenderRequest& renderRequest, const V3& /*position*/)
{
    if(_items.size() > 0 && _stub->_visible.val())
        return _stub->_render_command_composer->compose(renderRequest, *this);

    DrawingContext drawingContext(_stub->_shader_bindings, nullptr, std::move(_ubos), std::move(_ssbos));
    return drawingContext.toBindCommand();
}

bool RenderLayer::Snapshot::needsReload() const
{
    return _flag == RenderLayer::SNAPSHOT_FLAG_RELOAD || _stub->_shader_bindings->vertices().size() == 0;
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

RenderLayer::Snapshot RenderLayer::snapshot(RenderRequest& renderRequest) const
{
    return Snapshot(renderRequest, _stub);
}

const sp<Layer>& RenderLayer::layer() const
{
    return _stub->_layer;
}

sp<LayerContext> RenderLayer::makeContext(sp<Batch> batch, sp<ModelLoader> modelLoader, sp<Boolean> visible) const
{
    return _stub->makeLayerContext(std::move(batch), std::move(modelLoader), std::move(visible));
}

sp<Layer> RenderLayer::makeLayer(sp<ModelLoader> modelLoader, sp<Boolean> visible) const
{
    return sp<Layer>::make(makeContext(nullptr, std::move(modelLoader), std::move(visible)));
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
    : _resource_loader_context(resourceLoaderContext), _layers(factory.getBuilderList<Layer>(manifest, "layer")), _model_loader(std::move(modelLoader)), _shader(shader ? std::move(shader) : Shader::fromDocument(factory, manifest, resourceLoaderContext)),
      _scissor(factory.getBuilder<Vec4>(manifest, "scissor"))
{
}

sp<RenderLayer> RenderLayer::BUILDER::build(const Scope& args)
{
    const sp<RenderLayer> renderLayer = sp<RenderLayer>::make(_model_loader->build(args), _shader->build(args), _scissor->build(args), _resource_loader_context->renderController());
    for(const sp<Builder<Layer>>& i : _layers)
    {
        const sp<Layer> layer = i->build(args);
        layer->setContext(renderLayer->makeContext(nullptr, layer->modelLoader()));
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
