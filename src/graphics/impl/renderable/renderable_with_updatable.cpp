#include "graphics/impl/renderable/renderable_with_updatable.h"

#include "graphics/base/render_request.h"


namespace ark {

RenderableWithUpdatable::RenderableWithUpdatable(sp<Renderable> delegate, sp<Updatable> updatable)
    : Wrapper(std::move(delegate)), _updatable(std::move(updatable))
{
}

Renderable::StateBits RenderableWithUpdatable::updateState(const RenderRequest& renderRequest)
{
    StateBits stateBits = _wrapped->updateState(renderRequest);
    if(_updatable->update(renderRequest.timestamp()))
        return static_cast<StateBits>(RENDERABLE_STATE_DIRTY | stateBits);
    return stateBits;
}

Renderable::Snapshot RenderableWithUpdatable::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state)
{
    return _wrapped->snapshot(snapshotContext, renderRequest, state);
}

}
