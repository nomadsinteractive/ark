#include "graphics/base/render_layer_snapshot.h"

#include <ranges>

#include "core/util/log.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/layer_context_snapshot.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/inf/drawing_context_composer.h"

namespace ark {

class RenderLayerSnapshot::OnceGuard {
public:
    explicit OnceGuard(const RenderLayerSnapshot& snapshot)
        : _stub(const_cast<RenderLayerSnapshot&>(snapshot)._stub)
    {
        CHECK(_stub, "This method can be only called once");
    }
    ~OnceGuard()
    {
        _stub = nullptr;
    }

private:
    sp<RenderLayer::Stub>& _stub;
};

RenderLayerSnapshot::RenderLayerSnapshot(const RenderRequest& renderRequest, const sp<RenderLayer::Stub>& stub)
    : _stub(stub), _index_count(0), _vertices_dirty(false)
{
    if(_stub->_scissor)
    {
        _stub->_scissor->update(renderRequest.tick());
        _scissor = Rect(_stub->_scissor->val());
    }
}

sp<RenderCommand> RenderLayerSnapshot::compose(const RenderRequest& renderRequest, sp<DrawDecorator> drawDecorator) const
{
    const OnceGuard guard(*this);
    if(!_elements.empty() && _stub->_visible.val())
        return _stub->_drawing_context_composer->compose(renderRequest, *this).toRenderCommand(renderRequest, std::move(drawDecorator));

    DrawingContext drawingContext(_stub->_pipeline_bindings, _stub->_shader->takeBufferSnapshot(renderRequest, false));
    return drawingContext.toNoopCommand(renderRequest, std::move(drawDecorator));
}

DrawingContext RenderLayerSnapshot::toDrawingContext(const RenderRequest& renderRequest, Buffer::Snapshot vertices, Buffer::Snapshot indices, const uint32_t drawCount, DrawingParams params) const
{
    const OnceGuard guard(*this);
    DrawingContext drawingContext(_stub->_pipeline_bindings, _stub->_shader->takeBufferSnapshot(renderRequest, false), std::move(vertices), std::move(indices),
                                  drawCount, std::move(params));
    if(_stub->_scissor)
        drawingContext._scissor = _stub->_render_controller->renderEngine()->toRendererRect(_scissor, enums::COORDINATE_SYSTEM_LHS);

    return drawingContext;
}

bool RenderLayerSnapshot::needsReload() const
{
    return _vertices_dirty || _stub->_pipeline_bindings->vertices().size() == 0;
}

void RenderLayerSnapshot::addLayerContext(const RenderRequest& renderRequest, Vector<sp<LayerContext>>& layerContexts)
{
    for(auto iter = layerContexts.begin(); iter != layerContexts.end(); )
    {
        const sp<LayerContext>& layerContext = *iter;
        if(const OptionalVar<Boolean>& discarded = layerContext->discarded(); discarded.val())
        {
            addDiscardedLayerContext(layerContext);
            iter = layerContexts.erase(iter);
            _vertices_dirty = true;
        }
        else
        {
            _vertices_dirty = doAddLayerContext(renderRequest, layerContext) || _vertices_dirty;
            ++iter;
        }
    }
}

bool RenderLayerSnapshot::addDiscardedState(LayerContext& lc, const void* stateKey)
{
    if(const auto iter = lc._element_states.find(stateKey); iter != lc._element_states.end())
    {
        LayerContext::ElementState& elementState = iter->second;
        const bool v = static_cast<bool>(elementState._index);
        _elements_deleted.push_back(std::move(elementState));
        lc._element_states.erase(iter);
        return v;
    }
    return true;
}

void RenderLayerSnapshot::addDiscardedLayerContext(LayerContext& lc)
{
    for(auto& v : lc._element_states | std::views::values)
        _elements_deleted.push_back(std::move(v));
    lc._renderables.clear();
    lc._element_states.clear();
}

void RenderLayerSnapshot::addDiscardedLayerContexts(const Vector<sp<LayerContext>>& layerContexts)
{
    for(const sp<LayerContext>& lc : layerContexts)
        addDiscardedLayerContext(lc);
}

bool RenderLayerSnapshot::doAddLayerContext(const RenderRequest& renderRequest, LayerContext& layerContext)
{
    const PipelineLayout& shaderLayout = _stub->_shader->layout();

    _layer_context_snapshots.push_back(layerContext.snapshot(RenderLayer(_stub), renderRequest, shaderLayout));
    const LayerContextSnapshot& layerSnapshot = _layer_context_snapshots.back();

    const bool reload = needsReload();
    bool verticesDirty = layerContext.processNewCreated();

    for(auto iter = layerContext._renderables.begin(); iter != layerContext._renderables.end(); )
    {
        Renderable& renderable = iter->first;
        Renderable::State& s = iter->second;
        const Renderable::State newState = renderable.updateState(renderRequest);
        const bool visibilityChanged = s.contains(Renderable::RENDERABLE_STATE_VISIBLE) != newState.contains(Renderable::RENDERABLE_STATE_VISIBLE);
        s = {newState, static_cast<Renderable::StateBits>(s.bits() & (Renderable::RENDERABLE_STATE_NEW | Renderable::RENDERABLE_STATE_DISCARDED))};
        if(Renderable::State state = s; state.contains(Renderable::RENDERABLE_STATE_DISCARDED))
        {
            LOGD("delete: %p", &renderable);
            verticesDirty = true;
            addDiscardedState(layerContext, &renderable);
            iter = layerContext._renderables.erase(iter);
        }
        else
        {
            if(visibilityChanged)
                state.set(Renderable::RENDERABLE_STATE_NEW, true);
            if(reload)
                state.set(Renderable::RENDERABLE_STATE_DIRTY, true);
            if(state.contains(Renderable::RENDERABLE_STATE_VISIBLE))
                state.set(Renderable::RENDERABLE_STATE_VISIBLE, layerSnapshot._visible);
            if(s.contains(Renderable::RENDERABLE_STATE_NEW))
            {
                state.set(Renderable::RENDERABLE_STATE_DIRTY, true);
                s.set(Renderable::RENDERABLE_STATE_NEW, false);
            }

            const auto fiter = layerContext._element_states.find(&renderable);
            DASSERT(fiter != layerContext._element_states.end());
            _elements.emplace_back(renderable, layerSnapshot, fiter->second, Renderable::Snapshot{state});

            ++iter;
        }
    }

    return verticesDirty;
}

void RenderLayerSnapshot::snapshot(const RenderRequest& renderRequest)
{
    const bool reload = needsReload();
    for(Element& i : _elements)
    {
        const Renderable::Snapshot& snapshot = i.ensureSnapshot(renderRequest, reload);
        _index_count += snapshot._model->indexCount();
    }
}

RenderLayerSnapshot::Element::Element(Renderable& renderable, const LayerContextSnapshot& layerContext, LayerContext::ElementState& state, const Renderable::Snapshot& snapshot)
    : _renderable(renderable), _layer_context(layerContext), _element_state(state), _snapshot(snapshot)
{
}

const Renderable::Snapshot& RenderLayerSnapshot::Element::ensureSnapshot(const RenderRequest& renderRequest, const bool reload)
{
    if(reload)
        _snapshot._state.set(Renderable::RENDERABLE_STATE_DIRTY, true);
    _snapshot =_renderable.snapshot(_layer_context, renderRequest, _snapshot._state);
    ASSERT(_snapshot._model);
    _snapshot._position += _layer_context._position;
    _snapshot.applyVaryings(_layer_context._varyings);
    return _snapshot;
}

}
