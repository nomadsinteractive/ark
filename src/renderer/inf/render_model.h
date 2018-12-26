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
        RENDER_MODE_NONE,
        RENDER_MODE_LINES,
        RENDER_MODE_POINTS,
        RENDER_MODE_TRIANGLES,
        RENDER_MODE_TRIANGLE_STRIP,
        RENDER_MODE_COUNT,
    };

    RenderModel(Mode mode)
        : _mode(mode) {
    }
    virtual ~RenderModel() = default;

    Mode mode() const {
        return _mode;
    }

    virtual void initialize(ShaderBindings& bindings) = 0;
    virtual void start(ModelBuffer& buf, RenderController& renderController, const Layer::Snapshot& layerContext) = 0;
    virtual void load(ModelBuffer& buf, int32_t type, const V& size) = 0;

    virtual Metrics measure(int32_t type) { return Metrics(); }

protected:
    Mode _mode;
};

}

#endif
