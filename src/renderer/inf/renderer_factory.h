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

    virtual sp<RenderContext> initialize(Ark::RendererVersion version) = 0;

    virtual void onSurfaceCreated(RenderContext& renderContext) = 0;

    virtual sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) = 0;
    virtual sp<PipelineFactory> createPipelineFactory() = 0;
    virtual sp<RenderView> createRenderView(const sp<RenderContext>& glContext, const Viewport& viewport) = 0;
    virtual sp<Texture> createTexture(const sp<Recycler>& recycler, uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmap) = 0;
};

}

#endif
