#include "graphics/base/layer_context.h"

#include "core/ark.h"
#include "core/epi/disposed.h"
#include "core/base/notifier.h"
#include "core/util/holder_util.h"
#include "core/util/log.h"

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
    _renderable_created.emplace_back(std::move(renderable), std::move(isDirty), std::move(isDisposed));
    _reload_requested = true;
}

void LayerContext::clear()
{
    for(auto& [i, j] : _renderables)
        i._disposed = SafeVar<Boolean>(nullptr, true);
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

bool LayerContext::snapshot(RenderRequest& renderRequest, RenderLayerSnapshot& output)
{
    DPROFILER_TRACE("TakeSnapshot");

    bool needsReload = _render_batch->preSnapshot(renderRequest, *this, output) || _reload_requested;
    _render_batch->snapshot(renderRequest, *this, output);
    _render_done = _visible.val();
    _reload_requested = false;

    DPROFILER_LOG("Renderables", _renderables.size());
    return needsReload || _layer_type == Layer::TYPE_TRANSIENT;
}

bool LayerContext::ensureState(void* stateKey)
{
    const auto iter = _element_states.find(stateKey);
    if(iter == _element_states.end())
    {
        _element_states.insert(std::make_pair(stateKey, ElementState{0}));
        return true;
    }
    return false;
}

LayerContext::ElementState& LayerContext::addElementState(void* key)
{
    DASSERT(_element_states.find(key) == _element_states.end());
    return _element_states.insert(std::make_pair(key, ElementState{0})).first->second;
}

bool LayerContext::doPreSnapshot(const RenderRequest& renderRequest, RenderLayerSnapshot& output)
{
    bool needsReload = false;
    if(_renderable_created.size() > 0)
    {
        for(RenderableItem& i : _renderable_created)
        {
            LOGD("create: %p", i._renderable.get());
            addElementState(i._renderable.get());
            _renderables.emplace_back(std::move(i), Renderable::State(Renderable::RENDERABLE_STATE_NEW));
        }
        _renderable_created.clear();
        needsReload = true;
    }

    const uint64_t timestamp = renderRequest.timestamp();
    for(auto iter = _renderables.begin(); iter != _renderables.end(); )
    {
        const LayerContext::RenderableItem& i = iter->first;
        Renderable::State& state = iter->second;
        i._disposed.update(timestamp);
        state.setState(i._renderable->updateState(renderRequest));
        if(!state || state.hasState(Renderable::RENDERABLE_STATE_DISPOSED) || i._disposed.val())
        {
            LOGD("delete: %p", i._renderable.get());
            needsReload = true;
            output.addDisposedState(*this, i._renderable.get());
            iter = _renderables.erase(iter);
        }
        else
        {
            if(!state.hasState(Renderable::RENDERABLE_STATE_DIRTY) && i._updatable)
                state.setState(Renderable::RENDERABLE_STATE_DIRTY, i._updatable->update(timestamp));

            ++iter;
        }
    }
    return needsReload;
}

void LayerContext::doSnapshot(const RenderRequest& renderRequest, RenderLayerSnapshot& output)
{
    const PipelineInput& pipelineInput = output.pipelineInput();
    const bool visible = _visible.val();
    const bool needsReload = _position_changed || _render_done != visible || output.needsReload();
    Varyings::Snapshot defaultVaryingsSnapshot = _varyings ? _varyings->snapshot(pipelineInput, renderRequest.allocator()) : Varyings::Snapshot();

    for(auto& [i, j] : _renderables) {
        Renderable& renderable = i;
        Renderable::State state = j;
        if(needsReload)
            state.setState(Renderable::RENDERABLE_STATE_DIRTY, true);
        if(state.hasState(Renderable::RENDERABLE_STATE_VISIBLE))
            state.setState(Renderable::RENDERABLE_STATE_VISIBLE, visible);
        if(j.hasState(Renderable::RENDERABLE_STATE_NEW))
        {
            state.setState(Renderable::RENDERABLE_STATE_DIRTY, true);
            j.setState(Renderable::RENDERABLE_STATE_NEW, false);
        }
        Renderable::Snapshot snapshot = renderable.snapshot(pipelineInput, renderRequest, _position, state.stateBits());
        output.loadSnapshot(*this, snapshot, defaultVaryingsSnapshot);
        output.addSnapshot(*this, std::move(snapshot), &renderable);
    }
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
