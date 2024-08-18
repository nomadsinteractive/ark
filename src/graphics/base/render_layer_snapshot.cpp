#include "graphics/base/render_layer_snapshot.h"

#include "core/util/log.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/layer_context_snapshot.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

RenderLayerSnapshot::RenderLayerSnapshot(RenderRequest& renderRequest, const sp<RenderLayer::Stub>& stub)
    : _stub(stub), _index_count(0), _buffer_object(sp<BufferObject>::make(BufferObject{_stub->_shader->takeUBOSnapshot(renderRequest), _stub->_shader->takeSSBOSnapshot(renderRequest)}))
{
    _vertices_dirty = addLayerContext(renderRequest, _stub->_layer_context);

    if(_stub->_scissor && _stub->_scissor->update(renderRequest.timestamp()))
        _scissor = Rect(_stub->_scissor->val());
}

sp<RenderCommand> RenderLayerSnapshot::compose(const RenderRequest& renderRequest)
{
    if(!_droplets.empty() && _stub->_layer_context->visible().val())
        return _stub->_render_command_composer->compose(renderRequest, *this);

    DrawingContext drawingContext(_stub->_pipeline_bindings, _buffer_object, nullptr);
    return drawingContext.toBindCommand();
}

bool RenderLayerSnapshot::needsReload() const
{
    return _vertices_dirty || _stub->_pipeline_bindings->vertices().size() == 0;
}

const sp<PipelineInput>& RenderLayerSnapshot::pipelineInput() const
{
    return _stub->_shader->input();
}

void RenderLayerSnapshot::snapshot(RenderRequest& renderRequest, std::vector<sp<LayerContext>>& layerContexts)
{
    for(auto iter = layerContexts.begin(); iter != layerContexts.end(); )
    {
        const sp<LayerContext>& layerContext = *iter;
        const SafeVar<Boolean>& discarded = layerContext->discarded();
        if((!discarded && layerContext.unique()) || discarded.val())
        {
            addDiscardedLayerContext(layerContext);
            iter = layerContexts.erase(iter);
            _vertices_dirty = true;
        }
        else
        {
            _vertices_dirty = addLayerContext(renderRequest, layerContext) || _vertices_dirty;
            ++iter;
        }
    }
}

bool RenderLayerSnapshot::addDiscardedState(LayerContext& lc, void* stateKey)
{
    if(const auto iter = lc._element_states.find(stateKey); iter != lc._element_states.end())
    {
        const LayerContext::ElementState& elementState = iter->second;
        const bool v = static_cast<bool>(elementState._index);
        _item_deleted.push_back(elementState);
        lc._element_states.erase(iter);
        return v;
    }
    return true;
}

void RenderLayerSnapshot::addDiscardedLayerContext(LayerContext& lc)
{
    for(const auto& [k, v] : lc._element_states)
        _item_deleted.push_back(v);
    lc._renderables.clear();
    lc._element_states.clear();
}

void RenderLayerSnapshot::addDiscardedLayerContexts(const std::vector<sp<LayerContext>>& layerContexts)
{
    for(const sp<LayerContext>& lc : layerContexts)
        addDiscardedLayerContext(lc);
}

sp<RenderCommand> RenderLayerSnapshot::toRenderCommand(const RenderRequest& renderRequest, Buffer::Snapshot vertices, Buffer::Snapshot indices, uint32_t drawCount, DrawingParams params) const
{
    DrawingContext drawingContext(_stub->_pipeline_bindings, _buffer_object, _stub->_pipeline_bindings->attachments(), std::move(vertices), std::move(indices),
                                  drawCount, std::move(params));

    if(_stub->_scissor)
        drawingContext._scissor = _stub->_render_controller->renderEngine()->toRendererRect(_scissor);

    return drawingContext.toRenderCommand(renderRequest);
}

bool RenderLayerSnapshot::addLayerContext(RenderRequest& renderRequest, LayerContext& layerContext)
{
    const PipelineInput& pipelineInput = _stub->_shader->input();

    _layer_context_snapshots.push_back(layerContext.snapshot(RenderLayer(_stub), renderRequest, pipelineInput));
    const LayerContextSnapshot& layerSnapshot = _layer_context_snapshots.back();

    const bool reload = needsReload();
    bool verticesDirty = layerContext.processNewCreated();

    for(auto iter = layerContext._renderables.begin(); iter != layerContext._renderables.end(); )
    {
        Renderable& renderable = iter->first;
        Renderable::State& s = iter->second;
        s.reset(renderable.updateState(renderRequest));
        ASSERT(s);
        if(Renderable::State state = s; !state || state.has(Renderable::RENDERABLE_STATE_DISCARDED))
        {
            LOGD("delete: %p", &renderable);
            verticesDirty = true;
            addDiscardedState(layerContext, &renderable);
            iter = layerContext._renderables.erase(iter);
        }
        else
        {
            if(reload)
                state.set(Renderable::RENDERABLE_STATE_DIRTY, true);
            if(state.has(Renderable::RENDERABLE_STATE_VISIBLE))
                state.set(Renderable::RENDERABLE_STATE_VISIBLE, layerSnapshot._visible);
            if(s.has(Renderable::RENDERABLE_STATE_NEW))
            {
                state.set(Renderable::RENDERABLE_STATE_DIRTY, true);
                s.set(Renderable::RENDERABLE_STATE_NEW, false);
            }
            Renderable::Snapshot snapshot = renderable.snapshot(layerSnapshot, renderRequest, state.stateBits());
            DASSERT(snapshot._model);
            snapshot._position += layerSnapshot._position;
            snapshot.applyVaryings(layerSnapshot._varyings);
            if(!snapshot._model)
                snapshot._model = layerContext._model_loader->loadModel(snapshot._type);
            _index_count += snapshot._model->indexCount();

            const auto fiter = layerContext._element_states.find(&renderable);
            DASSERT(fiter != layerContext._element_states.end());
            _droplets.emplace_back(renderable, layerSnapshot, fiter->second, snapshot);

            ++iter;
        }
    }

    return verticesDirty;
}

void RenderLayerSnapshot::doSnapshot(const RenderRequest& renderRequest, const LayerContextSnapshot& layerSnapshot)
{
    for(Droplet& i : _droplets)
    {
        Renderable::Snapshot snapshot = i._renderable.snapshot(i._layer_context, renderRequest, i._snapshot._state.stateBits());
        snapshot._position += layerSnapshot._position;
        snapshot.applyVaryings(layerSnapshot._varyings);
        _index_count += snapshot._model->indexCount();
    }
}

RenderLayerSnapshot::Droplet::Droplet(Renderable& renderable, const LayerContextSnapshot& layerContext, LayerContext::ElementState& state, const Renderable::Snapshot& snapshot)
    : _renderable(renderable), _layer_context(layerContext), _element_state(state), _snapshot(snapshot)
{
}

const Renderable::Snapshot& RenderLayerSnapshot::Droplet::ensureDirtySnapshot(const RenderRequest& renderRequest)
{
    if(!_snapshot._state.has(Renderable::RENDERABLE_STATE_DIRTY))
    {
        _snapshot =_renderable.snapshot(_layer_context, renderRequest, static_cast<Renderable::StateBits>(_snapshot._state.stateBits() | Renderable::RENDERABLE_STATE_DIRTY));
        _snapshot._position += _layer_context._position;
        _snapshot.applyVaryings(_layer_context._varyings);
    }
    return _snapshot;
}

}
