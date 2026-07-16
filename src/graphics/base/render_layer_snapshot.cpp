#include "graphics/base/render_layer_snapshot.h"

#include <ranges>

#include "core/util/log.h"
#include "core/util/updatable_util.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/layer_context_snapshot.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_backend.h"
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
    : _stub(stub), _index_count(0), _vertices_dirty(false), _render_layer_dirty(UpdatableUtil::update(renderRequest.tick(), _stub->_visible, _stub->_scissor))
{
    if(_stub->_scissor)
        _scissor = Rect(_stub->_scissor->val());
}

sp<RenderCommand> RenderLayerSnapshot::compose(const RenderRequest& renderRequest, sp<DrawDecorator> drawDecorator)
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
    if(_stub->_is_dynamic_scissor && _stub->_scissor)
        drawingContext._scissor = _stub->_render_controller->renderBackend()->toRendererRect(_scissor, enums::COORDINATE_SYSTEM_LHS);

    return drawingContext;
}

bool RenderLayerSnapshot::verticesDirty() const
{
    return _vertices_dirty || _stub->_pipeline_bindings->vertices().size() == 0;
}

bool RenderLayerSnapshot::layersDirty() const
{
    if(_render_layer_dirty)
        return true;

    for(const LayerContextSnapshot& i : _layer_context_snapshots)
        if(i._dirty)
            return true;
    return false;
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

void RenderLayerSnapshot::addDiscardedLayerContext(LayerContext& lc)
{
    for(auto& v : lc._element_states | std::views::values)
        _elements_deleted.push_back(std::move(v));
    lc._renderables.clear();
    lc._element_states.clear();
}

bool RenderLayerSnapshot::doAddLayerContext(const RenderRequest& renderRequest, LayerContext& layerContext)
{
    const PipelineLayout& shaderLayout = _stub->_shader->layout();

    _layer_context_snapshots.push_back(layerContext.snapshot(renderRequest, shaderLayout));
    const LayerContextSnapshot& layerSnapshot = _layer_context_snapshots.back();

    const bool reload = verticesDirty();
    for(const LayerContext::RenderableState& i : layerContext._renderable_states)
    {
        Renderable::State state = i._state;
        if(reload)
            state.set(Renderable::RENDERABLE_STATE_DIRTY, true);
        _elements.emplace_back(*i._renderable, layerSnapshot, *i._element_state, Renderable::Snapshot{state});
    }

    for(const LayerContext::ElementState& i : layerContext._discarded_element_states)
        _elements_deleted.push_back(i);

    return layerContext._instances_dirty;
}

void RenderLayerSnapshot::snapshot(const RenderRequest& renderRequest)
{
    const bool reload = verticesDirty() || layersDirty();
    for(Element& i : _elements)
    {
        const Renderable::Snapshot& snapshot = i.ensureSnapshot(renderRequest, *this, reload);
        _index_count += snapshot._model->indexCount();
    }
}

RenderLayerSnapshot::Element::Element(Renderable& renderable, const LayerContextSnapshot& layerContext, LayerContext::ElementState& state, const Renderable::Snapshot& snapshot)
    : _renderable(renderable), _layer_context(layerContext), _element_state(state), _snapshot(snapshot)
{
}

const Renderable::Snapshot& RenderLayerSnapshot::Element::ensureSnapshot(const RenderRequest& renderRequest, const RenderLayerSnapshot& renderLayerSnapshot, const bool reload)
{
    if(reload)
        _snapshot._state.set(Renderable::RENDERABLE_STATE_DIRTY, true);
    _snapshot =_renderable.snapshot(renderLayerSnapshot, renderRequest, _snapshot._state);
    ASSERT(_snapshot._model);
    _snapshot._position += _layer_context._position;
    _snapshot.applyVaryings(_layer_context._varyings);
    return _snapshot;
}

}
