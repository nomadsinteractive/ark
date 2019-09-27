#include "graphics/base/layer_context.h"

#include "core/epi/disposed.h"
#include "core/epi/notifier.h"
#include "core/util/holder_util.h"

#include "graphics/base/layer.h"

namespace ark {

LayerContext::Item::Item(float x, float y, const sp<RenderObject>& renderObject)
    : _x(x), _y(y), _render_object(renderObject)
{
}

LayerContext::LayerContext(const sp<RenderModel>& renderModel, const sp<Notifier>& notifier, Layer::Type type)
    : _render_model(renderModel), _notifier(notifier), _layer_type(type), _render_requested(false)
{
}

void LayerContext::traverse(const Holder::Visitor& visitor)
{
    for(const sp<RenderObject>& i : _items)
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

void LayerContext::renderRequest(const V2& position)
{
    _render_requested = true;
    _position = position;
}

void LayerContext::drawRenderObject(float x, float y, const sp<RenderObject>& renderObject)
{
    _transient_items.emplace_back(x + _position.x(), y + _position.y(), renderObject);
}

void LayerContext::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed)
{
    DASSERT(renderObject);
    _items.push_back(renderObject, disposed, _notifier);
    _notifier->notify();
}

void LayerContext::clear()
{
    _items.clear();
    _notifier->notify();
}

void LayerContext::takeSnapshot(RenderLayer::Snapshot& output, MemoryPool& memoryPool)
{
    if(_render_requested)
    {
        bool notify = false;
        for(const Item& i : _transient_items)
            if(!i._render_object->isDisposed())
            {
                RenderObject::Snapshot snapshot = i._render_object->snapshot(memoryPool);
                snapshot._position = snapshot._position + V3(i._x, i._y, 0);
                output._items.push_back(std::move(snapshot));
                notify = true;
            }

        for(const sp<RenderObject>& i : _items)
        {
            if(i->isVisible())
            {
                RenderObject::Snapshot snapshot = i->snapshot(memoryPool);
                snapshot._position = snapshot._position + _position;
                output._items.push_back(std::move(snapshot));
            }
        }

        if(notify || _layer_type == Layer::TYPE_DYNAMIC || _layer_type == Layer::TYPE_TRANSIENT)
            _notifier->notify();
    }
    _transient_items.clear();
    _render_requested = false;
}

LayerContext::RenderObjectFilter::RenderObjectFilter(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed, const sp<Notifier>& notifier)
    : _disposed(disposed ? disposed : renderObject.as<Disposed>().cast<Boolean>()), _notifier(notifier)
{
}

FilterAction LayerContext::RenderObjectFilter::operator()(const sp<RenderObject>& renderObject) const
{
    if(renderObject->isDisposed() || (_disposed && _disposed->val()))
    {
        renderObject->dispose();
        _notifier->notify();
        return FILTER_ACTION_REMOVE;
    }
    return FILTER_ACTION_NONE;
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
