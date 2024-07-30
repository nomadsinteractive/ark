#include "graphics/base/render_layer_snapshot.h"

#include "core/util/log.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/layer_context_snapshot.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

RenderLayerSnapshot::RenderLayerSnapshot(RenderRequest& renderRequest, const sp<RenderLayer::Stub>& stub)
    : _stub(stub), _index_count(0)
{
    _needs_reload = addLayerContext(renderRequest, _stub->_layer_context);

    _ubos = _stub->_shader->takeUBOSnapshot(renderRequest);
    _ssbos = _stub->_shader->takeSSBOSnapshot(renderRequest);

    if(_stub->_scissor && _stub->_scissor->update(renderRequest.timestamp()))
        _scissor = Rect(_stub->_scissor->val());
}

sp<RenderCommand> RenderLayerSnapshot::compose(const RenderRequest& renderRequest)
{
    if(_droplets.size() > 0 && _stub->_layer_context->visible().val())
        return _stub->_render_command_composer->compose(renderRequest, *this);

    DrawingContext drawingContext({_stub->_shader_bindings, std::move(_ubos), std::move(_ssbos)}, nullptr);
    return drawingContext.toBindCommand();
}

bool RenderLayerSnapshot::needsReload() const
{
    return _needs_reload || _stub->_shader_bindings->vertices().size() == 0;
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
        const SafeVar<Boolean>& disposed = layerContext->discarded();
        if((!disposed && layerContext.unique()) || disposed.val())
        {
            addDiscardedLayerContext(layerContext);
            iter = layerContexts.erase(iter);
            _needs_reload = true;
        }
        else
        {
            _needs_reload = addLayerContext(renderRequest, layerContext) || _needs_reload;
            ++iter;
        }
    }
}

bool RenderLayerSnapshot::addDisposedState(LayerContext& lc, void* stateKey)
{
    const auto iter = lc._element_states.find(stateKey);
    if(iter != lc._element_states.end())
    {
        const LayerContext::ElementState& elementState = iter->second;
        bool v = static_cast<bool>(elementState._index);
        _item_deleted.push_back(elementState);
        lc._element_states.erase(iter);
        return v;
    }
    return true;
}

void RenderLayerSnapshot::addDiscardedLayerContext(LayerContext& lc)
{
    for(const auto& [i, j] : lc._element_states)
        _item_deleted.push_back(j);
    lc._renderables.clear();
    lc._element_states.clear();
}

void RenderLayerSnapshot::addDiscardedLayerContexts(const std::vector<sp<LayerContext>>& layerContexts)
{
    for(LayerContext& lc : layerContexts)
        addDiscardedLayerContext(lc);
}

sp<RenderCommand> RenderLayerSnapshot::toRenderCommand(const RenderRequest& renderRequest, Buffer::Snapshot vertices, Buffer::Snapshot indices, uint32_t drawCount, DrawingParams params)
{
    DrawingContext drawingContext({_stub->_shader_bindings, std::move(_ubos), std::move(_ssbos)}, _stub->_shader_bindings->attachments(), std::move(vertices), std::move(indices),
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
    bool dirty = layerContext.processNewCreated();

    for(auto iter = layerContext._renderables.begin(); iter != layerContext._renderables.end(); )
    {
        Renderable& renderable = iter->first;
        Renderable::State& s = iter->second;
        s.setState(renderable.updateState(renderRequest));
        ASSERT(s);
        if(Renderable::State state = s; !state || state.hasState(Renderable::RENDERABLE_STATE_DISCARDED))
        {
            LOGD("delete: %p", &renderable);
            dirty = true;
            addDisposedState(layerContext, &renderable);
            iter = layerContext._renderables.erase(iter);
        }
        else
        {
            if(reload)
                state.setState(Renderable::RENDERABLE_STATE_DIRTY, true);
            if(state.hasState(Renderable::RENDERABLE_STATE_VISIBLE))
                state.setState(Renderable::RENDERABLE_STATE_VISIBLE, layerSnapshot._visible);
            if(s.hasState(Renderable::RENDERABLE_STATE_NEW))
            {
                state.setState(Renderable::RENDERABLE_STATE_DIRTY, true);
                s.setState(Renderable::RENDERABLE_STATE_NEW, false);
            }
            Renderable::Snapshot snapshot = renderable.snapshot(layerSnapshot, renderRequest, state.stateBits());
            snapshot._position += layerSnapshot._position;
            snapshot.applyVaryings(layerSnapshot._varyings);
            if(!snapshot._model)
                snapshot._model = layerContext._model_loader->loadModel(snapshot._type);
            _index_count += snapshot._model->indexCount();
            auto fiter = layerContext._element_states.find(&renderable);
            DASSERT(fiter != layerContext._element_states.end());
            _droplets.emplace_back(renderable, layerSnapshot, fiter->second, snapshot);

            ++iter;
        }
    }

    return dirty;
}

RenderLayerSnapshot::Droplet::Droplet(Renderable& renderable, const LayerContextSnapshot& layerContext, LayerContext::ElementState& state, const Renderable::Snapshot& snapshot)
    : _renderable(renderable), _layer_context(layerContext), _element_state(state), _snapshot(snapshot)
{
}

const Renderable::Snapshot& RenderLayerSnapshot::Droplet::ensureDirtySnapshot(const RenderRequest& renderRequest)
{
    if(!_snapshot._state.hasState(Renderable::RENDERABLE_STATE_DIRTY))
    {
        _snapshot =_renderable.snapshot(_layer_context, renderRequest, static_cast<Renderable::StateBits>(_snapshot._state.stateBits() | Renderable::RENDERABLE_STATE_DIRTY));
        _snapshot._position += _layer_context._position;
        _snapshot.applyVaryings(_layer_context._varyings);
    }
    return _snapshot;
}

}
