#include "graphics/impl/renderable/renderable_with_visible.h"

#include "graphics/base/layer_context_snapshot.h"
#include "graphics/base/render_request.h"

namespace ark {

RenderableWithVisible::RenderableWithVisible(sp<Renderable> delegate, sp<Boolean> visible)
    : Wrapper(std::move(delegate)), _visible(std::move(visible))
{
}


Renderable::StateBits RenderableWithVisible::updateState(const RenderRequest& renderRequest)
{
    State state = _wrapped->updateState(renderRequest);
    if(_visible->update(renderRequest.timestamp()) && !_visible->val())
        state.setState(RENDERABLE_STATE_VISIBLE, false);
    return state.stateBits();
}

Renderable::Snapshot RenderableWithVisible::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state)
{
    return _wrapped->snapshot(snapshotContext, renderRequest, state);
}

}
