#ifndef ARK_RENDERER_INF_RENDERER_FACTORY_H_
#define ARK_RENDERER_INF_RENDERER_FACTORY_H_

#include "core/ark.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/camera.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/texture.h"

namespace ark {

class ARK_API RendererFactory {
public:
    virtual ~RendererFactory() = default;

    virtual sp<RenderEngineContext> initialize(Ark::RendererVersion version) = 0;

    virtual void onSurfaceCreated(RenderEngineContext& renderContext) = 0;

    virtual sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) = 0;
    virtual sp<Camera::Delegate> createCamera(Ark::RendererCoordinateSystem cs) = 0;
    virtual sp<Framebuffer> createFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask) = 0;
    virtual sp<PipelineFactory> createPipelineFactory() = 0;
    virtual sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) = 0;
    virtual sp<Texture::Delegate> createTexture(const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader) = 0;
};

}

#endif
