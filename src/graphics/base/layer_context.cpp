#include "graphics/base/layer_context.h"

#include "core/ark.h"
#include "core/epi/disposed.h"
#include "core/base/notifier.h"
#include "core/util/holder_util.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_object.h"
#include "graphics/impl/render_batch/render_batch_impl.h"

#include "renderer/base/model.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/shader.h"

namespace ark {

LayerContext::RenderableItem::RenderableItem(sp<Renderable> renderable, sp<Updatable> updatable, sp<Boolean> disposed)
    : _renderable(std::move(renderable)), _updatable(std::move(updatable)), _disposed(std::move(disposed), false)
{
    ASSERT(_renderable);
}

LayerContext::RenderableItem::operator Renderable&() const
{
    return *_renderable;
}

LayerContext::LayerContext(sp<RenderBatch> batch, sp<ModelLoader> models, sp<Boolean> visible, sp<Boolean> disposed, sp<Varyings> varyings)
    : _render_batch(batch ? std::move(batch) : sp<RenderBatch>::make<RenderBatchImpl>()), _model_loader(std::move(models)), _visible(visible ? sp<Visibility>::make(std::move(visible)) : nullptr, true),
      _disposed(disposed ? std::move(disposed) : nullptr, false), _varyings(std::move(varyings)), _layer_type(Layer::TYPE_DYNAMIC), _reload_requested(false), _render_done(false), _position_changed(false)
{
    DASSERT(_render_batch);
}

void LayerContext::traverse(const Holder::Visitor& visitor)
{
    if(_layer_type != Layer::TYPE_TRANSIENT)
        for(const auto& i : _renderables)
            HolderUtil::visit(i.first._renderable, visitor);
}

SafeVar<Boolean>& LayerContext::visible()
{
    return _visible;
}

const SafeVar<Boolean>& LayerContext::visible() const
{
    return _visible;
}

const SafeVar<Boolean>& LayerContext::disposed() const
{
    return _disposed;
}

const sp<ModelLoader>& LayerContext::modelLoader() const
{
    return _model_loader;
}

void LayerContext::setModelLoader(sp<ModelLoader> modelLoader)
{
    _model_loader = std::move(modelLoader);
}

Layer::Type LayerContext::layerType() const
{
    return _layer_type;
}

void LayerContext::renderRequest(const V3& position)
{
    _position_changed = _position != position;
    if(_position_changed)
        _position = position;
}

void LayerContext::add(sp<Renderable> renderable, sp<Updatable> isDirty, sp<Boolean> isDisposed)
{
    ASSERT(renderable);
    _renderable_emplaced.emplace_back(std::move(renderable), std::move(isDirty), std::move(isDisposed));
    _reload_requested = true;
}

void LayerContext::clear()
{
    _renderables.clear();
    _reload_requested = true;
}

const sp<Varyings>& LayerContext::varyings() const
{
    return _varyings;
}

void LayerContext::setVaryings(sp<Varyings> varyings)
{
    _varyings = std::move(varyings);
}

bool LayerContext::preSnapshot(RenderRequest& renderRequest)
{
    bool needsReload = _renderable_emplaced.size() > 0;
    if(needsReload)
    {
        for(RenderableItem& i : _renderable_emplaced)
            _renderables.emplace_back(std::move(i), Renderable::State());
        _renderable_emplaced.clear();
    }

    needsReload = _render_batch->preSnapshot(renderRequest, *this) || needsReload || _reload_requested;
    _reload_requested = false;

    return needsReload || _layer_type == Layer::TYPE_TRANSIENT;
}

void LayerContext::snapshot(RenderRequest& renderRequest, RenderLayerSnapshot& output)
{
    DPROFILER_TRACE("TakeSnapshot");

    _render_batch->snapshot(renderRequest, *this, output);
    _render_done = _visible.val();

    DPROFILER_LOG("Renderables", _renderables.size());
}

LayerContext::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, Layer::Type layerType)
    : _layer(factory.getBuilder<Layer>(manifest, Constants::Attributes::LAYER)),
      _render_layer(_layer ? nullptr : factory.getBuilder<RenderLayer>(manifest, Constants::Attributes::RENDER_LAYER)), _layer_type(layerType)
{
    CHECK(_layer || _render_layer, "LayerContext must be associated with one Layer or RenderLayer");
}

sp<LayerContext> LayerContext::BUILDER::build(const Scope& args)
{
    if(_layer)
        return _layer->build(args)->context();
    const sp<RenderLayer> renderLayer = _render_layer->build(args);
    if(_layer_type != Layer::TYPE_UNSPECIFIED)
        return renderLayer->makeLayerContext();
    return renderLayer->context();
}

}
