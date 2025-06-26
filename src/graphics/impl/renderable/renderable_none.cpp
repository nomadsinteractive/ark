#include "graphics/impl/renderable/renderable_none.h"

#include "graphics/base/render_request.h"

namespace ark {

Renderable::State RenderableNone::updateState(const RenderRequest& renderRequest)
{
    return {RENDERABLE_STATE_NONE};
}

Renderable::Snapshot RenderableNone::snapshot(const LayerContextSnapshot& /*snapshotContext*/, const RenderRequest& /*renderRequest*/, State /*state*/)
{
    return {RENDERABLE_STATE_NONE};
}

}
