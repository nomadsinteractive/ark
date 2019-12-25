#include "graphics/base/layer_context.h"

#include "core/epi/disposed.h"
#include "core/epi/notifier.h"
#include "core/util/holder_util.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_object.h"

#include "renderer/base/shader.h"

namespace ark {

LayerContext::Item::Item(const V3& position, const sp<RenderObject>& renderObject)
    : _position(position), _render_object(renderObject)
{
}

LayerContext::LayerContext(const sp<RenderModel>& renderModel, const sp<Notifier>& notifier, Layer::Type type)
    : _render_model(renderModel), _notifier(notifier), _layer_type(type), _render_requested(false)
{
}

void LayerContext::traverse(const Holder::Visitor& visitor)
{
    if(_layer_type != Layer::TYPE_TRANSIENT)
        for(const sp<Renderable>& i : _renderables)
            HolderUtil::visit(i, visitor);
}

const sp<RenderModel>& LayerContext::renderModel() const
{
    return _render_model;
}

Layer::Type LayerContext::layerType() const
{
    return _layer_type;
}

void LayerContext::renderRequest(const V3& position)
{
    _render_requested = true;
    _position = position;
}

void LayerContext::add(const sp<Renderable>& renderable, const sp<Boolean>& disposed)
{
    DASSERT(renderable);
    _renderables.push_back(renderable, disposed);
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
    if(_render_requested)
    {
        const sp<PipelineInput> pipelineInput = output._stub->_shader->input();
        bool notify = false;

        for(const sp<Renderable>& i : _renderables)
        {
            Renderable::Snapshot snapshot = i->snapshot(pipelineInput, renderRequest);
            if(snapshot._disposed)
                notify = true;
            else
            {
                snapshot._position = snapshot._position + _position;
                output._items.emplace_back(std::move(snapshot));
            }
        }

        if(notify || _layer_type == Layer::TYPE_TRANSIENT)
            _notifier->notify();
    }
    _render_requested = false;
}

LayerContext::RenderableFilter::RenderableFilter(const sp<Renderable>& /*renderable*/, const sp<Boolean>& disposed)
    : _disposed(disposed)
{
}

FilterAction LayerContext::RenderableFilter::operator()(const sp<Renderable>& /*renderable*/) const
{
    return (_disposed && _disposed->val()) ? FILTER_ACTION_REMOVE_AFTER : FILTER_ACTION_NONE;
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
