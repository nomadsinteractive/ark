#ifndef ARK_RENDERER_INF_RENDER_MODEL_H_
#define ARK_RENDERER_INF_RENDER_MODEL_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/metrics.h"
#include "graphics/inf/renderable.h"

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
    RenderModel(const sp<ModelLoader>& modelLoader = nullptr)
        : _model_loader(modelLoader) {
    }

    virtual ~RenderModel() = default;

    virtual sp<ShaderBindings> makeShaderBindings(const Shader& shader) = 0;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) = 0;

    virtual void start(DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot) = 0;
    virtual void load(VertexStream& buf, const Renderable::Snapshot& snapshot) = 0;

    virtual Metrics measure(int32_t type) { return Metrics(); }

    const sp<ModelLoader>& modelLoader() const {
        return _model_loader;
    }

private:
    sp<ModelLoader> _model_loader;

};

}

#endif
