#ifndef ARK_RENDERER_INF_RENDER_MODEL_H_
#define ARK_RENDERER_INF_RENDER_MODEL_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer.h"
#include "graphics/base/metrics.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API RenderModel {
public:
    enum Mode {
        RENDER_MODE_NONE = -1,
        RENDER_MODE_LINES,
        RENDER_MODE_POINTS,
        RENDER_MODE_TRIANGLES,
        RENDER_MODE_TRIANGLE_STRIP,
        RENDER_MODE_COUNT,
    };

    virtual ~RenderModel() = default;

    virtual sp<ShaderBindings> makeShaderBindings(const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout) = 0;
    virtual void postSnapshot(RenderController& renderController, Layer::Snapshot& snapshot) = 0;

    virtual void start(ModelBuffer& buf, RenderController& renderController, const Layer::Snapshot& snapshot) = 0;
    virtual void load(ModelBuffer& buf, int32_t type, const V& size) = 0;

    virtual Metrics measure(int32_t type) { return Metrics(); }

};

}

#endif
