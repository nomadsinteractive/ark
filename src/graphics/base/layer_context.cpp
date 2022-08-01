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

LayerContext::Item::Item(const sp<Renderable>& renderable, const sp<Boolean>& disposed)
    : _renderable(renderable), _disposed(disposed, false)
{
    DASSERT(_renderable);
}

LayerContext::LayerContext(sp<ModelLoader> models, sp<Varyings> varyings, Layer::Type type)
    : _model_loader(std::move(models)), _varyings(std::move(varyings)), _layer_type(type), _reload_requested(false), _render_requested(false), _render_done(false), _position_changed(false)
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
    _renderable_emplaced.emplace_back(renderable, disposed);
    _reload_requested = true;
}

void LayerContext::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed)
{
    add(renderObject, disposed ? disposed : renderObject->disposed().as<Boolean>());
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

bool LayerContext::preSnapshot(const RenderRequest& renderRequest)
{
    bool needsReload = _renderable_emplaced.size() > 0;
    if(needsReload)
    {
        const std::vector<Item> emplaced(std::move(_renderable_emplaced));
        _renderables.insert(_renderables.end(), emplaced.begin(), emplaced.end());
    }

    const uint64_t timestamp = renderRequest.timestamp();

    for(auto iter = _renderables.begin(); iter != _renderables.end(); )
    {
        Item& i = *iter;
        i._disposed.update(timestamp);
        i._state = i._renderable->updateState(renderRequest);
        if((i._state & Renderable::RENDERABLE_STATE_DISPOSED) || i._disposed.val())
        {
            needsReload = true;
            iter = _renderables.erase(iter);
        }
        else
            ++iter;
    }

    return needsReload || _reload_requested || _layer_type == Layer::TYPE_TRANSIENT;
}

void LayerContext::takeSnapshot(RenderLayer::Snapshot& output, const RenderRequest& renderRequest)
{
    DPROFILER_TRACE("TakeSnapshot");
    const sp<PipelineInput>& pipelineInput = output._stub->_shader->input();
    const bool needsReload = _position_changed || _render_done != _render_requested || output.needsReload();
    const bool hasDefaultVaryings = static_cast<bool>(_varyings);
    const Varyings::Snapshot defaultVaryingsSnapshot = hasDefaultVaryings ? _varyings->snapshot(pipelineInput, renderRequest.allocator()) : Varyings::Snapshot();

    for(auto iter = _renderables.begin(); iter != _renderables.end(); )
    {
        Item& i = *iter;
        if(needsReload)
            Renderable::setState(i._state, Renderable::RENDERABLE_STATE_DIRTY, true);
        if(i._state & Renderable::RENDERABLE_STATE_VISIBLE)
            Renderable::setState(i._state, Renderable::RENDERABLE_STATE_VISIBLE, _render_requested);
        Renderable::Snapshot snapshot = i._renderable->snapshot(pipelineInput, renderRequest, _position, i._state);
        snapshot._model = _model_loader->loadModel(snapshot._type);
        if(hasDefaultVaryings && !snapshot._varyings)
            snapshot._varyings = defaultVaryingsSnapshot;
        output._index_count += snapshot._model->indexCount();
        output._items.push_back(std::move(snapshot));
        ++iter;
    }

    _render_done = _render_requested;
    _render_requested = false;

    DPROFILER_LOG("Renderables", _renderables.size());
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
