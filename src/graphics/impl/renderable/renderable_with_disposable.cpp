#include "graphics/impl/renderable/renderable_with_disposable.h"

#include "graphics/base/render_request.h"

namespace ark {

RenderableWithDisposable::RenderableWithDisposable(sp<Renderable> delegate, sp<Boolean> disposed)
    : Wrapper(std::move(delegate)), _discarded(std::move(disposed))
{
}

Renderable::StateBits RenderableWithDisposable::updateState(const RenderRequest& renderRequest)
{
    StateBits stateBits = _wrapped->updateState(renderRequest);
    _discarded->update(renderRequest.timestamp());
    if(_discarded->val())
        return static_cast<StateBits>(RENDERABLE_STATE_DISCARDED | stateBits);
    return stateBits;
}

Renderable::Snapshot RenderableWithDisposable::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state)
{
    return _wrapped->snapshot(snapshotContext, renderRequest, postTranslate, state);
}

}
