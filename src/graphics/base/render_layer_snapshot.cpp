#include "graphics/base/render_layer_snapshot.h"

#include "graphics/base/layer_context.h"

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
    DPROFILER_TRACE("Snapshot");

    _needs_reload = _stub->_layer_context->snapshot(renderRequest, *this);

    _ubos = _stub->_shader->takeUBOSnapshot(renderRequest);
    _ssbos = _stub->_shader->takeSSBOSnapshot(renderRequest);

    if(_stub->_scissor && _stub->_scissor->update(renderRequest.timestamp()))
        _scissor = Rect(_stub->_scissor->val());

    DPROFILER_LOG("NeedsReload", _needs_reload);
}

sp<RenderCommand> RenderLayerSnapshot::compose(const RenderRequest& renderRequest)
{
    if(_items.size() > 0 && _stub->_layer_context->visible().val())
        return _stub->_render_command_composer->compose(renderRequest, *this);

    DrawingContext drawingContext(_stub->_shader_bindings, nullptr, std::move(_ubos), std::move(_ssbos));
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
        const SafeVar<Boolean>& disposed = layerContext->disposed();
        if((!disposed && layerContext.unique()) || disposed.val())
        {
            addDisposedLayerContext(layerContext);
            iter = layerContexts.erase(iter);
            _needs_reload = true;
        }
        else
        {
            _needs_reload = layerContext->snapshot(renderRequest, *this) || _needs_reload;
            ++iter;
        }
    }
}

void RenderLayerSnapshot::addSnapshot(LayerContext& lc, Renderable::Snapshot snapshot, void* stateKey)
{
    auto iter = lc._element_states.find(stateKey);
    DASSERT(iter != lc._element_states.end());
    _items.emplace_back(iter->second, std::move(snapshot));
}

void RenderLayerSnapshot::addDisposedState(LayerContext& lc, void* stateKey)
{
    const auto iter = lc._element_states.find(stateKey);
    if(iter != lc._element_states.end())
    {
        _item_deleted.push_back(iter->second);
        lc._element_states.erase(iter);
    }
}

void RenderLayerSnapshot::addDisposedLayerContext(LayerContext& lc)
{
    for(const auto& [i, j] : lc._element_states)
        _item_deleted.push_back(j);
    lc._renderables.clear();
    lc._element_states.clear();
}

void RenderLayerSnapshot::addDisposedLayerContexts(const std::vector<sp<LayerContext>>& layerContexts)
{
    for(LayerContext& lc : layerContexts)
        addDisposedLayerContext(lc);
}

bool RenderLayerSnapshot::addLayerContext(const RenderRequest& renderRequest, LayerContext& layerContext)
{
    bool dirty = layerContext.doPreSnapshot(renderRequest, *this);
    layerContext.doSnapshot(renderRequest, *this);
    return dirty;
}

sp<RenderCommand> RenderLayerSnapshot::toRenderCommand(const RenderRequest& renderRequest, Buffer::Snapshot vertices, Buffer::Snapshot indices, DrawingContextParams::Parameters params)
{
    DrawingContext drawingContext(_stub->_shader_bindings, _stub->_shader_bindings->attachments(), std::move(_ubos), std::move(_ssbos), std::move(vertices), std::move(indices),
                                  std::move(params));

    if(_stub->_scissor)
        drawingContext._scissor = _stub->_render_controller->renderEngine()->toRendererRect(_scissor);

    return drawingContext.toRenderCommand(renderRequest);
}

void RenderLayerSnapshot::loadSnapshot(const LayerContext& lc, Renderable::Snapshot& snapshot, const Varyings::Snapshot& defaultVaryingsSnapshot)
{
    snapshot.applyVaryings(defaultVaryingsSnapshot);
    if(!snapshot._model)
        snapshot._model = lc._model_loader->loadModel(snapshot._type);
    _index_count += snapshot._model->indexCount();
}

RenderLayerSnapshot::SnapshotWithState::SnapshotWithState(LayerContext::ElementState& state, Renderable::Snapshot snapshot)
    : _state(state), _snapshot(std::move(snapshot))
{
}

}
