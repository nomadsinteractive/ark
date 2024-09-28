#pragma once

#include "graphics/forwarding.h"
#include "graphics/inf/render_batch.h"

namespace ark {

class RenderBatchImpl final : public RenderBatch {
public:

    std::vector<sp<LayerContext>>& snapshot(const RenderRequest& renderRequest) override;

    void addLayerContext(sp<LayerContext> layerContext);

private:
    std::vector<sp<LayerContext>> _layer_contexts;
};

}
