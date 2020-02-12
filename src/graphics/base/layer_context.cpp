#include "graphics/base/layer_context.h"

#include "core/epi/disposed.h"
#include "core/epi/notifier.h"
#include "core/util/holder_util.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_object.h"

#include "renderer/base/shader.h"

#include "core/util/log.h"
#include "renderer/base/pipeline_input.h"

namespace ark {

LayerContext::Item::Item(const sp<Renderable>& renderable, const sp<Boolean>& disposed)
    : _renderable(renderable), _disposed(disposed, false)
{
    DASSERT(_renderable);
}

LayerContext::LayerContext(const sp<ModelLoader>& models, const sp<Notifier>& notifier, Layer::Type type)
    : _model_loader(models), _notifier(notifier), _layer_type(type), _render_requested(false), _render_done(false), _position_changed(false)
{
}

void LayerContext::traverse(const Holder::Visitor& visitor)
{
    if(_layer_type != Layer::TYPE_TRANSIENT)
        for(const Item& i : _renderables)
            HolderUtil::visit(i._renderable, visitor);
}

const sp<ModelLoader>& LayerContext::modelLoader() const
{
    return _model_loader;
}

Layer::Type LayerContext::layerType() const
{
    return _layer_type;
}

void LayerContext::renderRequest(const V3& position)
{
    _render_requested = true;
    _position_changed = _position != position;
    if(_position_changed)
        _position = position;
}

void LayerContext::add(const sp<Renderable>& renderable, const sp<Boolean>& disposed)
{
    DASSERT(renderable);
    _renderables.emplace_back(renderable, disposed);
    _notifier->notify();
}

void LayerContext::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed)
{
    add(renderObject, disposed ? disposed : renderObject->disposed().as<Boolean>());
}

void LayerContext::clear()
{
    _renderables.clear();
    _notifier->notify();
}

void LayerContext::takeSnapshot(RenderLayer::Snapshot& output, const RenderRequest& renderRequest)
{
    bool notify = false;
    const sp<PipelineInput>& pipelineInput = output._stub->_shader->input();

    for(auto iter = _renderables.begin(); iter != _renderables.end(); )
    {
        const Item& i = *iter;
        Renderable::Snapshot snapshot = i._disposed.val() ? Renderable::Snapshot() : i._renderable->snapshot(pipelineInput, renderRequest);
        snapshot._position += _position;
        if(snapshot._disposed || snapshot._type == -1)
        {
            notify = true;
            iter = _renderables.erase(iter);
        }
        else
        {
            snapshot._dirty = snapshot._dirty || _position_changed || _render_done != _render_requested;
            snapshot._visible = _render_requested && snapshot._visible;
            output._items.emplace_back(std::move(snapshot));
            ++iter;
        }
    }

    if(notify || _layer_type == Layer::TYPE_TRANSIENT)
        _notifier->notify();

    _render_done = _render_requested;
    _render_requested = false;
}

LayerContext::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, Layer::Type layerType)
    : _layer(factory.getBuilder<Layer>(manifest, Constants::Attributes::LAYER)),
      _render_layer(_layer ? nullptr : factory.getBuilder<RenderLayer>(manifest, Constants::Attributes::RENDER_LAYER)), _layer_type(layerType)
{
    DCHECK(_layer || _render_layer, "LayerContext must be associated with one Layer or RenderLayer");
}

sp<LayerContext> LayerContext::BUILDER::build(const Scope& args)
{
    if(_layer)
        return _layer->build(args)->context();
    const sp<RenderLayer> renderLayer = _render_layer->build(args);
    if(_layer_type != Layer::TYPE_UNSPECIFIED)
        return renderLayer->makeContext(_layer_type);
    return renderLayer->context();
}

}
