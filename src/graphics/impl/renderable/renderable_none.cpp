#include "graphics/impl/renderable/renderable_none.h"

#include "graphics/base/render_request.h"

namespace ark {

Renderable::StateBits RenderableNone::updateState(const RenderRequest& renderRequest)
{
    return Renderable::RENDERABLE_STATE_NONE;
}

Renderable::Snapshot RenderableNone::snapshot(const LayerContextSnapshot& /*snapshotContext*/, const RenderRequest& /*renderRequest*/, const V3& /*postTranslate*/, StateBits /*state*/)
{
    return Renderable::Snapshot(Renderable::RENDERABLE_STATE_NONE);
}

}
