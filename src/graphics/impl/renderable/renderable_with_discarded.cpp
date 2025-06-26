#include "graphics/impl/renderable/renderable_with_discarded.h"

#include "graphics/base/render_request.h"

namespace ark {

RenderableWithDiscarded::RenderableWithDiscarded(sp<Renderable> delegate, sp<Boolean> discarded)
    : Wrapper(std::move(delegate)), _discarded(std::move(discarded))
{
}

Renderable::State RenderableWithDiscarded::updateState(const RenderRequest& renderRequest)
{
    const State state = _wrapped->updateState(renderRequest);
    _discarded->update(renderRequest.timestamp());
    if(_discarded->val())
        return state | RENDERABLE_STATE_DISCARDED;
    return state;
}

Renderable::Snapshot RenderableWithDiscarded::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, const State state)
{
    return _wrapped->snapshot(snapshotContext, renderRequest, state);
}

}
