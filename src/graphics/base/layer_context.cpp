#include "graphics/base/layer_context.h"

#include "core/ark.h"
#include "core/epi/disposed.h"
#include "core/base/notifier.h"
#include "core/util/holder_util.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_object.h"

#include "renderer/base/model.h"
#include "renderer/base/shader.h"

#include "core/util/log.h"
#include "renderer/base/pipeline_input.h"

namespace ark {

LayerContext::RenderableItem::RenderableItem(sp<Renderable> renderable, sp<Updatable> updatable, sp<Boolean> disposed)
    : _renderable(std::move(renderable)), _updatable(std::move(updatable)), _disposed(std::move(disposed), false)
{
    ASSERT(_renderable);
}

LayerContext::LayerContext(sp<RenderableBatch> batch, sp<ModelLoader> models, sp<Boolean> visible, sp<Boolean> disposed, sp<Varyings> varyings)
    : _batch(batch ? std::move(batch) : sp<RenderableBatch>::make<DefaultBatch>()), _model_loader(std::move(models)), _visible(visible ? sp<Visibility>::make(std::move(visible)) : nullptr, true),
      _disposed(disposed ? std::move(disposed) : nullptr, false), _varyings(std::move(varyings)), _layer_type(Layer::TYPE_DYNAMIC), _reload_requested(false), _render_done(false), _position_changed(false)
{
}

void LayerContext::traverse(const Holder::Visitor& visitor)
{
    if(_layer_type != Layer::TYPE_TRANSIENT)
        for(const RenderableItem& i : _renderables)
            HolderUtil::visit(i._renderable, visitor);
}

SafeVar<Visibility>& LayerContext::visible()
{
    return _visible;
}

const SafeVar<Visibility>& LayerContext::visible() const
{
    return _visible;
}

const SafeVar<Disposed>& LayerContext::disposed() const
{
    return _disposed;
}

bool LayerContext::isDisposed() const
{
    return _disposed.val();
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
        const std::vector<RenderableItem> emplaced(std::move(_renderable_emplaced));
        _renderables.insert(_renderables.end(), emplaced.begin(), emplaced.end());
    }

    needsReload = _batch->preSnapshot(renderRequest, *this) || needsReload || _reload_requested;
    _reload_requested = false;

    return needsReload || _layer_type == Layer::TYPE_TRANSIENT;
}

void LayerContext::snapshot(RenderRequest& renderRequest, RenderLayerSnapshot& output)
{
    DPROFILER_TRACE("TakeSnapshot");

    _batch->snapshot(renderRequest, *this, output);
    _render_done = _visible.val();

    DPROFILER_LOG("Renderables", _renderables.size());
}

bool LayerContext::DefaultBatch::preSnapshot(const RenderRequest& renderRequest, LayerContext& lc)
{
    bool needsReload = false;
    const uint64_t timestamp = renderRequest.timestamp();

    for(auto iter = lc._renderables.begin(); iter != lc._renderables.end(); )
    {
        LayerContext::RenderableItem& i = *iter;
        i._disposed.update(timestamp);
        i._state = i._renderable->updateState(renderRequest);
        if(!i._state || i._state.hasState(Renderable::RENDERABLE_STATE_DISPOSED) || i._disposed.val())
        {
            needsReload = true;
            iter = lc._renderables.erase(iter);
        }
        else
        {
            if(!i._state.hasState(Renderable::RENDERABLE_STATE_DIRTY) && i._updatable)
                i._state.setState(Renderable::RENDERABLE_STATE_DIRTY, i._updatable->update(timestamp));

            ++iter;
        }
    }
    return needsReload;
}

void LayerContext::DefaultBatch::snapshot(const RenderRequest& renderRequest, const LayerContext& lc, RenderLayerSnapshot& output)
{
    const PipelineInput& pipelineInput = output.pipelineInput();
    const bool visible = lc._visible.val();
    const bool needsReload = lc._position_changed || lc._render_done != visible || output.needsReload();
    const bool hasDefaultVaryings = static_cast<bool>(lc._varyings);
    const Varyings::Snapshot defaultVaryingsSnapshot = hasDefaultVaryings ? lc._varyings->snapshot(pipelineInput, renderRequest.allocator()) : Varyings::Snapshot();

    for(auto iter = lc._renderables.begin(); iter != lc._renderables.end(); )
    {
        const LayerContext::RenderableItem& i = *iter;
        Renderable::State state = i._state;
        if(needsReload)
            state.setState(Renderable::RENDERABLE_STATE_DIRTY, true);
        if(state.hasState(Renderable::RENDERABLE_STATE_VISIBLE))
            state.setState(Renderable::RENDERABLE_STATE_VISIBLE, visible);
        Renderable::Snapshot snapshot = i._renderable->snapshot(pipelineInput, renderRequest, lc._position, state.stateBits());
        if(hasDefaultVaryings && !snapshot._varyings)
            snapshot._varyings = defaultVaryingsSnapshot;
        output.addSnapshot(lc, std::move(snapshot));
        ++iter;
    }
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
        return renderLayer->makeContext();
    return renderLayer->context();
}

}
