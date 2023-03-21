#ifndef ARK_GRAPHICS_IMPL_RENDER_BATCH_RENDER_BATCH_IMPL_H_
#define ARK_GRAPHICS_IMPL_RENDER_BATCH_RENDER_BATCH_IMPL_H_

#include "graphics/forwarding.h"
#include "graphics/inf/render_batch.h"

namespace ark {

class RenderBatchImpl : public RenderBatch {
public:

    virtual bool preSnapshot(const RenderRequest& renderRequest, LayerContext& lc, RenderLayerSnapshot& output) override;
    virtual void snapshot(const RenderRequest& renderRequest, LayerContext& lc, RenderLayerSnapshot& output) override;

};

}

#endif
