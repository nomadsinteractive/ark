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
        const LayerContext::RenderableItem& i = iter->first;
        Renderable::State& state = iter->second;
        i._disposed.update(timestamp);
        state = i._renderable->updateState(renderRequest);
        if(!state || state.hasState(Renderable::RENDERABLE_STATE_DISPOSED) || i._disposed.val())
        {
            needsReload = true;
            iter = lc._renderables.erase(iter);
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

void RenderBatchImpl::snapshot(const RenderRequest& renderRequest, const LayerContext& lc, RenderLayerSnapshot& output)
{
    lc.doSnapshot(lc._renderables, renderRequest, output);
}

}
