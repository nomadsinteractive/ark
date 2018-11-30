#ifndef ARK_RENDERER_INF_RENDERER_FACTORY_H_
#define ARK_RENDERER_INF_RENDERER_FACTORY_H_

#include "core/ark.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API RendererFactory {
public:
    virtual ~RendererFactory() = default;

    virtual void initialize(GLContext& glContext) = 0;

    virtual void setGLVersion(Ark::RendererVersion version, GLContext& glContext) = 0;

    virtual sp<RenderView> createRenderView(const sp<GLContext>& glContext, const Viewport& viewport) = 0;
    virtual sp<PipelineFactory> createPipelineFactory() = 0;
};

}

#endif
