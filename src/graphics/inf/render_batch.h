#ifndef ARK_GRAPHICS_INF_RENDER_BATCH_H_
#define ARK_GRAPHICS_INF_RENDER_BATCH_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API RenderBatch {
public:
    virtual ~RenderBatch() = default;

    virtual bool preSnapshot(const RenderRequest& renderRequest, LayerContext& lc, RenderLayerSnapshot& output) = 0;
    virtual void snapshot(const RenderRequest& renderRequest, LayerContext& lc, RenderLayerSnapshot& output) = 0;

};

}

#endif
