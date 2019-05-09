#include "graphics/base/layer_context.h"

#include "core/epi/disposed.h"
#include "core/epi/notifier.h"

#include "graphics/base/layer.h"

namespace ark {

LayerContext::Item::Item(float x, float y, const sp<RenderObject>& renderObject)
    : _x(x), _y(y), _render_object(renderObject)
{
}

LayerContext::LayerContext(const sp<RenderModel>& renderModel, const sp<Notifier>& notifier)
    : _render_model(renderModel), _notifier(notifier), _render_requested(false)
{
}

const sp<RenderModel>& LayerContext::renderModel() const
{
    return _render_model;
}

void LayerContext::renderRequest(const V2& position)
{
    _render_requested = true;
    _position = position;
}

void LayerContext::draw(float x, float y, const sp<RenderObject>& renderObject)
{
    _transient_items.emplace_back(x, y, renderObject);
}

void LayerContext::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed)
{
    DASSERT(renderObject);
    _items.push_back(renderObject, disposed ? sp<Disposed>::make(disposed) : renderObject.as<Disposed>(), _notifier);
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
                snapshot._position = snapshot._position + V3(i._x, i._y, 0) + _position;
                output._items.push_back(std::move(snapshot));
                notify = true;
            }
        if(notify)
            _notifier->notify();

        for(const sp<RenderObject>& i : _items)
        {
            RenderObject::Snapshot snapshot = i->snapshot(memoryPool);
            snapshot._position = snapshot._position + _position;
            output._items.push_back(std::move(snapshot));
        }
    }
    _transient_items.clear();
    _render_requested = false;
}

LayerContext::RenderObjectFilter::RenderObjectFilter(const sp<RenderObject>& /*renderObject*/, const sp<Disposed>& disposed, const sp<Notifier>& notifier)
    : _disposed(disposed), _notifier(notifier)
{
}

FilterAction LayerContext::RenderObjectFilter::operator()(const sp<RenderObject>& renderObject) const
{
    if(renderObject->isDisposed() || (_disposed && _disposed->isDisposed()))
    {
        renderObject->dispose();
        _notifier->notify();
        return FILTER_ACTION_REMOVE;
    }
    return FILTER_ACTION_NONE;
}

LayerContext::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, bool makeContext)
    : _layer(factory.getBuilder<Layer>(manifest, Constants::Attributes::LAYER)),
      _render_layer(_layer ? nullptr : factory.getBuilder<RenderLayer>(manifest, Constants::Attributes::RENDER_LAYER)), _make_context(makeContext)
{
    DCHECK(_layer || _render_layer, "RenderObject must be associated with one Layer or RenderLayer");
}

sp<LayerContext> LayerContext::BUILDER::build(const sp<Scope>& args)
{
    if(_layer)
        return _layer->build(args)->context();
    const sp<RenderLayer> renderLayer = _render_layer->build(args);
    if(_make_context)
        return renderLayer->makeContext();
    return renderLayer->context();
}

}
