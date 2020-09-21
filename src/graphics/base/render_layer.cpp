#include "graphics/base/render_layer.h"

#include "core/base/observer.h"
#include "core/epi/notifier.h"

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
#include "renderer/inf/animation.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

RenderLayer::Stub::Stub(const sp<ModelLoader>& modelLoader, const sp<Shader>& shader, const sp<Vec4>& scissor, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _model_loader(modelLoader), _shader(shader), _scissor(scissor), _render_controller(resourceLoaderContext->renderController()), _render_command_composer(_model_loader->makeRenderCommandComposer()),
      _shader_bindings(_render_command_composer->makeShaderBindings(_shader, _render_controller, _model_loader->renderMode())), _notifier(sp<Notifier>::make()),
      _dirty(_notifier->createDirtyFlag()), _layer(sp<Layer>::make(sp<LayerContext>::make(_model_loader, _notifier, Layer::TYPE_DYNAMIC))), _stride(shader->input()->getStream(0).stride())
{
    _model_loader->initialize(_shader_bindings);
    DCHECK(!_scissor || _shader_bindings->pipelineBindings()->hasFlag(PipelineBindings::FLAG_DYNAMIC_SCISSOR, PipelineBindings::FLAG_DYNAMIC_SCISSOR_BITMASK), "RenderLayer has a scissor while its Shader has no FLAG_DYNAMIC_SCISSOR set");
}

sp<LayerContext> RenderLayer::Stub::makeLayerContext(Layer::Type layerType)
{
    const sp<LayerContext> layerContext = sp<LayerContext>::make(_model_loader, _notifier, layerType);
    _layer_contexts.push_back(layerContext, _notifier);
    return layerContext;
}

RenderLayer::Snapshot::Snapshot(RenderRequest& renderRequest, const sp<Stub>& stub)
    : _stub(stub)
{
    Layer::Type combined = Layer::TYPE_UNSPECIFIED;

    _stub->_layer->context()->takeSnapshot(*this, renderRequest);
    for(const sp<LayerContext>& i : stub->_layer_contexts)
    {
        i->takeSnapshot(*this, renderRequest);
        DWARN(combined != Layer::TYPE_STATIC || i->layerType() != Layer::TYPE_DYNAMIC, "Combining static and dynamic layers together leads to low efficiency");
        if(combined != Layer::TYPE_DYNAMIC)
            combined = i->layerType();
    }

    _stub->_model_loader->postSnapshot(_stub->_render_controller, *this);
    _stub->_render_command_composer->postSnapshot(_stub->_render_controller, *this);

    _ubos = _stub->_shader->takeUBOSnapshot(renderRequest);
    _ssbos = _stub->_shader->takeSSBOSnapshot(renderRequest);

    if(_stub->_scissor && _stub->_scissor->update(renderRequest.timestamp()))
        _scissor = Rect(_stub->_scissor->val());

    bool dirty = _stub->_dirty->val();
    if(combined != Layer::TYPE_STATIC)
        _flag = dirty ? SNAPSHOT_FLAG_RELOAD : SNAPSHOT_FLAG_DYNAMIC_UPDATE;
    else
        _flag = dirty ? SNAPSHOT_FLAG_STATIC_MODIFIED : SNAPSHOT_FLAG_STATIC_REUSE;
}

sp<RenderCommand> RenderLayer::Snapshot::render(const RenderRequest& renderRequest, const V3& /*position*/)
{
    if(_items.size() > 0)
        return _stub->_render_command_composer->compose(renderRequest, *this);

    DrawingContext drawingContext(_stub->_shader_bindings, nullptr, std::move(_ubos), std::move(_ssbos));
    return drawingContext.toBindCommand();
}

RenderLayer::RenderLayer(const sp<ModelLoader>& modelLoader, const sp<Shader>& shader, const sp<Vec4>& scissor, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : RenderLayer(sp<Stub>::make(modelLoader, shader, scissor, resourceLoaderContext))
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

sp<LayerContext> RenderLayer::makeContext(Layer::Type layerType) const
{
    return _stub->makeLayerContext(layerType);
}

sp<Layer> RenderLayer::makeLayer(Layer::Type layerType) const
{
    return sp<Layer>::make(makeContext(layerType));
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
    : _resource_loader_context(resourceLoaderContext), _model_loader(std::move(modelLoader)), _shader(shader ? std::move(shader) : Shader::fromDocument(factory, manifest, resourceLoaderContext)),
      _scissor(factory.getBuilder<Vec4>(manifest, "scissor"))
{
}

sp<RenderLayer> RenderLayer::BUILDER::build(const Scope& args)
{
    return sp<RenderLayer>::make(_model_loader->build(args), _shader->build(args), _scissor->build(args), _resource_loader_context);
}

RenderLayer::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext)
{
}

sp<Renderer> RenderLayer::RENDERER_BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

RenderLayer::LayerContextFilter::LayerContextFilter(const sp<LayerContext>& /*item*/, const sp<Notifier>& notifier)
    : _notifier(notifier)
{
}

FilterAction RenderLayer::LayerContextFilter::operator()(const sp<LayerContext>& item) const
{
    if(item.unique())
    {
        _notifier->notify();
        return FILTER_ACTION_REMOVE;
    }
    return FILTER_ACTION_NONE;
}

}
