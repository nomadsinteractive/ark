#pragma once

#include "graphics/inf/render_batch.h"

namespace ark {

class RenderBatchPostProcess final : public RenderBatch {
public:
    RenderBatchPostProcess();

    Vector<sp<LayerContext>>& snapshot(const RenderRequest& renderRequest) override;

private:
    Vector<sp<LayerContext>> _contexts;
    sp<LayerContext> _layer_context;
};

}
