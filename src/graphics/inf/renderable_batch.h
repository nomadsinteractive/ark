#ifndef ARK_GRAPHICS_INF_RENDERABLE_BATCH_H_
#define ARK_GRAPHICS_INF_RENDERABLE_BATCH_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API RenderableBatch {
public:
    virtual ~RenderableBatch() = default;

    virtual bool preSnapshot(const RenderRequest& renderRequest, LayerContext& lc) = 0;
    virtual void snapshot(const RenderRequest& renderRequest, const LayerContext& lc, RenderLayerSnapshot& output) = 0;

};

}

#endif
