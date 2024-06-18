#include "graphics/impl/renderable/renderable_with_transform.h"

#include "graphics/base/render_request.h"

namespace ark {

RenderableWithTransform::RenderableWithTransform(sp<Renderable> delegate, sp<Mat4> transform)
    : Wrapper(std::move(delegate)), _transform(std::move(transform))
{
}

Renderable::StateBits RenderableWithTransform::updateState(const RenderRequest& renderRequest)
{
    const bool dirty = _transform->update(renderRequest.timestamp());
    const StateBits stateBits = _wrapped->updateState(renderRequest);
    return dirty ? static_cast<StateBits>(stateBits | RENDERABLE_STATE_DIRTY) : stateBits;
}

Renderable::Snapshot RenderableWithTransform::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state)
{
    Snapshot snapshot = _wrapped->snapshot(snapshotContext, renderRequest, state);
    snapshot._position = MatrixUtil::transform(_transform->val(), snapshot._position);
    return snapshot;
}

}
