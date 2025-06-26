#include "graphics/impl/renderable/renderable_with_visible.h"

#include "graphics/base/layer_context_snapshot.h"
#include "graphics/base/render_request.h"

namespace ark {

RenderableWithVisible::RenderableWithVisible(sp<Renderable> delegate, sp<Boolean> visible)
    : Wrapper(std::move(delegate)), _visible(std::move(visible))
{
}


Renderable::State RenderableWithVisible::updateState(const RenderRequest& renderRequest)
{
    State state = _wrapped->updateState(renderRequest);
    if(_visible->update(renderRequest.timestamp()) && !_visible->val())
        state.set(RENDERABLE_STATE_VISIBLE, false);
    return static_cast<StateBits>(state.bits());
}

Renderable::Snapshot RenderableWithVisible::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, const State state)
{
    return _wrapped->snapshot(snapshotContext, renderRequest, state);
}

}
