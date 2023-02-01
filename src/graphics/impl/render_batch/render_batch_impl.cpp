#include "graphics/impl/render_batch/render_batch_impl.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_request.h"
#include "graphics/base/render_layer_snapshot.h"

namespace ark {

bool RenderBatchImpl::preSnapshot(const RenderRequest& renderRequest, LayerContext& lc)
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

void RenderBatchImpl::snapshot(const RenderRequest& renderRequest, const LayerContext& lc, RenderLayerSnapshot& output)
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

}
