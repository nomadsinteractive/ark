#ifndef ARK_RENDERER_INF_RENDER_MODEL_H_
#define ARK_RENDERER_INF_RENDER_MODEL_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer.h"
#include "graphics/base/metrics.h"
#include "graphics/base/render_object.h"

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

    virtual sp<ShaderBindings> makeShaderBindings(RenderController& renderController, const sp<PipelineLayout>& pipelineLayout) = 0;
    virtual void postSnapshot(RenderController& renderController, Layer::Snapshot& snapshot) = 0;

    virtual void start(ModelBuffer& buf, const Layer::Snapshot& snapshot) = 0;
    virtual void load(ModelBuffer& buf, const RenderObject::Snapshot& snapshot) = 0;

    virtual Metrics measure(int32_t type) { return Metrics(); }

};

}

#endif
