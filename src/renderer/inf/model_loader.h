#ifndef ARK_RENDERER_INF_MODEL_LOADER_H_
#define ARK_RENDERER_INF_MODEL_LOADER_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_model.h"

namespace ark {

class ARK_API ModelLoader {
public:
    ModelLoader(RenderModel::Mode renderMode)
        : _render_mode(renderMode) {
    }
    virtual ~ModelLoader() = default;

    RenderModel::Mode renderMode() const {
        return _render_mode;
    }

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() = 0;

    virtual void initialize(ShaderBindings& shaderBindings) = 0;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) = 0;

    virtual Model load(int32_t type) = 0;

private:
    RenderModel::Mode _render_mode;
};

}

#endif
