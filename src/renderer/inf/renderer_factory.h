#ifndef ARK_RENDERER_INF_RENDERER_FACTORY_H_
#define ARK_RENDERER_INF_RENDERER_FACTORY_H_

#include "core/ark.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class ARK_API RendererFactory {
public:
    virtual ~RendererFactory() = default;

    virtual void initialize(GLContext& glContext) = 0;

    virtual void setGLVersion(Ark::RendererVersion version, GLContext& glContext) = 0;

    virtual sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Buffer::Uploader>& uploader) = 0;
    virtual sp<RenderView> createRenderView(const sp<GLContext>& glContext, const Viewport& viewport) = 0;
    virtual sp<PipelineFactory> createPipelineFactory() = 0;
};

}

#endif
