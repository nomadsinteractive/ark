#include "graphics/base/layer_context.h"

#include "core/ark.h"
#include "core/epi/disposed.h"
#include "core/base/notifier.h"
#include "core/util/holder_util.h"
#include "core/util/log.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_object.h"
#include "graphics/impl/renderable/renderable_with_disposable.h"
#include "graphics/impl/renderable/renderable_with_updatable.h"
#include "graphics/impl/render_batch/render_batch_impl.h"

#include "renderer/base/model.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/shader.h"

namespace ark {

static sp<Renderable> makeRenderable(sp<Renderable> renderable, sp<Updatable> updatable, sp<Boolean> disposed)
{
    ASSERT(renderable);
    if(updatable)
        renderable = sp<RenderableWithUpdatable>::make(std::move(renderable), std::move(updatable));
    if(disposed)
        renderable = sp<RenderableWithDisposable>::make(std::move(renderable), std::move(disposed));
    return renderable;
}

LayerContext::LayerContext(sp<ModelLoader> models, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> disposed, sp<Varyings> varyings)
    : _model_loader(std::move(models)), _position(position), _visible(std::move(visible), true), _disposed(disposed ? std::move(disposed) : nullptr, false),
      _varyings(std::move(varyings)), _layer_type(Layer::TYPE_DYNAMIC), _reload_requested(false), _render_done(false)
{
}

bool LayerContext::update(uint64_t timestamp)
{
    return _timestamp.update(timestamp);
}

void LayerContext::traverse(const Holder::Visitor& visitor)
{
    if(_layer_type != Layer::TYPE_TRANSIENT)
        for(const auto& i : _renderables)
            HolderUtil::visit(i.first, visitor);
}

const SafeVar<Vec3>& LayerContext::position() const
{
    return _position;
}

void LayerContext::setPosition(sp<Vec3> position)
{
    _position.reset(std::move(position));
    _timestamp.markDirty();
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

void LayerContext::add(sp<Renderable> renderable, sp<Updatable> isDirty, sp<Boolean> isDisposed)
{
    ASSERT(renderable);
    _renderable_created.push_back(makeRenderable(std::move(renderable), std::move(isDirty), std::move(isDisposed)));
}

void LayerContext::clear()
{
    for(auto& [i, j] : _renderables)
        j = Renderable::RENDERABLE_STATE_DISPOSED;
}

void LayerContext::dispose()
{
    _disposed = SafeVar<Boolean>(nullptr, true);
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

    bool needsReload = output.addLayerContext(renderRequest, *this);
    _render_done = _visible.val();

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
        for(sp<Renderable>& i : _renderable_created)
        {
            LOGD("create: %p", i.get());
            addElementState(i.get());
            _renderables.emplace_back(std::move(i), Renderable::State(Renderable::RENDERABLE_STATE_NEW));
        }
        _renderable_created.clear();
        needsReload = true;
    }

    for(auto iter = _renderables.begin(); iter != _renderables.end(); )
    {
        const sp<Renderable>& i = iter->first;
        Renderable::State& state = iter->second;
        state.setState(i->updateState(renderRequest));
        ASSERT(state);
        if(!state || state.hasState(Renderable::RENDERABLE_STATE_DISPOSED))
        {
            LOGD("delete: %p", i.get());
            needsReload = true;
            output.addDisposedState(*this, i.get());
            iter = _renderables.erase(iter);
        }
        else
            ++iter;
    }
    return needsReload;
}

void LayerContext::doSnapshot(const RenderRequest& renderRequest, RenderLayerSnapshot& output)
{
    const PipelineInput& pipelineInput = output.pipelineInput();
    const bool visible = _visible.val();
    const bool needsReload = _render_done != visible || output.needsReload();
    Varyings::Snapshot defaultVaryingsSnapshot = _varyings ? _varyings->snapshot(pipelineInput, renderRequest.allocator()) : Varyings::Snapshot();

    for(auto& [i, j] : _renderables) {
        Renderable& renderable = i;
        Renderable::State state = j;
        if(state)
        {
            if(needsReload)
                state.setState(Renderable::RENDERABLE_STATE_DIRTY, true);
            if(state.hasState(Renderable::RENDERABLE_STATE_VISIBLE))
                state.setState(Renderable::RENDERABLE_STATE_VISIBLE, visible);
            if(j.hasState(Renderable::RENDERABLE_STATE_NEW))
            {
                state.setState(Renderable::RENDERABLE_STATE_DIRTY, true);
                j.setState(Renderable::RENDERABLE_STATE_NEW, false);
            }
            Renderable::Snapshot snapshot = renderable.snapshot(pipelineInput, renderRequest, _position.val(), state.stateBits());
            output.loadSnapshot(*this, snapshot, defaultVaryingsSnapshot);
            output.addSnapshot(*this, std::move(snapshot), &renderable);
        }
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
        return renderLayer->addLayerContext();
    return renderLayer->context();
}

}
