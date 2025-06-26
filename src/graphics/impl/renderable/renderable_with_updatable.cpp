#include "graphics/impl/renderable/renderable_with_updatable.h"

#include "graphics/base/render_request.h"


namespace ark {

RenderableWithUpdatable::RenderableWithUpdatable(sp<Renderable> delegate, sp<Updatable> updatable)
    : Wrapper(std::move(delegate)), _updatable(std::move(updatable))
{
}

Renderable::State RenderableWithUpdatable::updateState(const RenderRequest& renderRequest)
{
    const State state = _wrapped->updateState(renderRequest);
    if(_updatable->update(renderRequest.timestamp()))
        return state | RENDERABLE_STATE_DIRTY;
    return state;
}

Renderable::Snapshot RenderableWithUpdatable::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, const State state)
{
    return _wrapped->snapshot(snapshotContext, renderRequest, state);
}

}
