#include "graphics/impl/render_batch/render_batch_impl.h"

#include "graphics/base/render_layer_snapshot.h"

namespace ark {

std::vector<sp<LayerContext>>& RenderBatchImpl::snapshot(const RenderRequest& renderRequest)
{
    return _layer_contexts;
}

void RenderBatchImpl::addLayerContext(sp<LayerContext> layerContext)
{
    _layer_contexts.push_back(std::move(layerContext));
}

}
