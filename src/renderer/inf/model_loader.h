#ifndef ARK_RENDERER_INF_MODEL_LOADER_H_
#define ARK_RENDERER_INF_MODEL_LOADER_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/base/render_layer.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API ModelLoader {
public:
    enum RenderMode {
        RENDER_MODE_NONE = -1,
        RENDER_MODE_LINES,
        RENDER_MODE_POINTS,
        RENDER_MODE_TRIANGLES,
        RENDER_MODE_TRIANGLE_STRIP,
        RENDER_MODE_COUNT,
    };

    ModelLoader(RenderMode renderMode);
    virtual ~ModelLoader() = default;

    RenderMode renderMode() const;

    class ARK_API Importer {
    public:
        virtual ~Importer() = default;

        virtual Model import(const document& manifest, const Rect& uvBounds) = 0;
    };

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() = 0;

    virtual void initialize(ShaderBindings& shaderBindings) = 0;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) = 0;

    virtual Model loadModel(int32_t type) = 0;

private:
    RenderMode _render_mode;
};

}

#endif
