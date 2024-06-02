#include "graphics/base/layer_context.h"

#include "core/ark.h"
#include "core/base/notifier.h"
#include "core/traits/expendable.h"
#include "core/util/holder_util.h"
#include "core/util/log.h"
#include "core/util/updatable_util.h"

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
    : _model_loader(std::move(models)), _position(position), _visible(std::move(visible), true), _discarded(disposed ? std::move(disposed) : nullptr, false),
      _varyings(std::move(varyings)), _layer_type(Layer::TYPE_DYNAMIC), _reload_requested(false)
{
}

bool LayerContext::update(uint64_t timestamp)
{
    return _timestamp.update(timestamp);
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
    return _discarded;
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

void LayerContext::add(sp<Renderable> renderable, sp<Updatable> isDirty, sp<Boolean> discarded)
{
    ASSERT(renderable);
    _renderable_created.push_back(makeRenderable(std::move(renderable), std::move(isDirty), std::move(discarded)));
}

void LayerContext::clear()
{
    for(auto& [i, j] : _renderables)
        j = Renderable::RENDERABLE_STATE_DISCARDED;
}

void LayerContext::dispose()
{
    _discarded = SafeVar<Boolean>(nullptr, true);
}

const sp<Varyings>& LayerContext::varyings() const
{
    return _varyings;
}

void LayerContext::setVaryings(sp<Varyings> varyings)
{
    _varyings = std::move(varyings);
}

void LayerContext::markDirty()
{
    _timestamp.markDirty();
}

bool LayerContext::processNewCreated()
{
    if(_renderable_created.size() == 0)
        return false;

    for(sp<Renderable>& i : _renderable_created)
    {
        LOGD("create: %p", i.get());
        addElementState(i.get());
        _renderables.emplace_back(std::move(i), Renderable::State(Renderable::RENDERABLE_STATE_NEW));
    }
    _renderable_created.clear();
    return true;
}

LayerContext::Snapshot LayerContext::snapshot(RenderRequest& renderRequest, const PipelineInput& pipelineInput) const
{
    bool dirty = UpdatableUtil::update(renderRequest.timestamp(), _position, _visible, _discarded, _varyings);
    Varyings::Snapshot varyings = _varyings ? _varyings->snapshot(pipelineInput, renderRequest.allocator()) : Varyings::Snapshot();
    return Snapshot{dirty, _position.val(), _visible.val(), _discarded.val(), varyings};
}

LayerContext::ElementState& LayerContext::addElementState(void* key)
{
    DASSERT(_element_states.find(key) == _element_states.end());
    return _element_states.insert(std::make_pair(key, ElementState{})).first->second;
}

LayerContext::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, Layer::Type layerType)
    : _layer(factory.getBuilder<Layer>(manifest, constants::LAYER)),
      _render_layer(_layer ? nullptr : factory.getBuilder<RenderLayer>(manifest, constants::RENDER_LAYER)), _layer_type(layerType)
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
