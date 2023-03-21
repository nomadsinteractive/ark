#include "graphics/impl/render_batch/render_batch_impl.h"

#include "graphics/base/layer_context.h"

namespace ark {

bool RenderBatchImpl::preSnapshot(const RenderRequest& renderRequest, LayerContext& lc, RenderLayerSnapshot& output)
{
    return lc.doPreSnapshot(renderRequest, output);
}

void RenderBatchImpl::snapshot(const RenderRequest& renderRequest, LayerContext& lc, RenderLayerSnapshot& output)
{
    lc.doSnapshot(renderRequest, output);
}

}
