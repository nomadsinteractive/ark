#pragma once

#include "core/base/wrapper.h"

#include "graphics/forwarding.h"
#include "graphics/inf/render_batch.h"

namespace ark {

class RenderBatchWithTranslation : public Wrapper<RenderBatch>, public RenderBatch {
public:
    RenderBatchWithTranslation(sp<RenderBatch> renderBatch, sp<Vec3> translation);

    virtual bool preSnapshot(const RenderRequest& renderRequest, LayerContext& lc, RenderLayerSnapshot& output) override;
    virtual void snapshot(const RenderRequest& renderRequest, LayerContext& lc, RenderLayerSnapshot& output) override;

private:
    sp<Vec3> _translation;

};

}
