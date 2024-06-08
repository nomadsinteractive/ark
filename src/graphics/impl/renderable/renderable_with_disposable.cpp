#include "graphics/impl/renderable/renderable_with_disposable.h"

#include "graphics/base/render_request.h"

namespace ark {

RenderableWithDiscarded::RenderableWithDiscarded(sp<Renderable> delegate, sp<Boolean> disposed)
    : Wrapper(std::move(delegate)), _discarded(std::move(disposed))
{
}

Renderable::StateBits RenderableWithDiscarded::updateState(const RenderRequest& renderRequest)
{
    StateBits stateBits = _wrapped->updateState(renderRequest);
    _discarded->update(renderRequest.timestamp());
    if(_discarded->val())
        return static_cast<StateBits>(RENDERABLE_STATE_DISCARDED | stateBits);
    return stateBits;
}

Renderable::Snapshot RenderableWithDiscarded::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state)
{
    return _wrapped->snapshot(snapshotContext, renderRequest, state);
}

}
