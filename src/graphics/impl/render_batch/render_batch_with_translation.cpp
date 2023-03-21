#include "graphics/impl/render_batch/render_batch_with_translation.h"

#include "core/inf/variable.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_request.h"

namespace ark {

RenderBatchWithTranslation::RenderBatchWithTranslation(sp<RenderBatch> renderBatch, sp<Vec3> translation)
    : Wrapper(std::move(renderBatch)), _translation(std::move(translation))
{
}

bool RenderBatchWithTranslation::preSnapshot(const RenderRequest& renderRequest, LayerContext& lc, RenderLayerSnapshot& output)
{
    bool dirty = _translation->update(renderRequest.timestamp());
    lc._position += _translation->val();
    return _wrapped->preSnapshot(renderRequest, lc, output) || dirty;
}

void RenderBatchWithTranslation::snapshot(const RenderRequest& renderRequest, LayerContext& lc, RenderLayerSnapshot& output)
{
    _wrapped->snapshot(renderRequest, lc, output);
}

}
