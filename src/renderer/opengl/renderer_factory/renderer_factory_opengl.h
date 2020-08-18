#ifndef ARK_RENDERER_OPENGL_RENDERER_FACTORY_RENDERER_FACTORY_OPENGL_H_
#define ARK_RENDERER_OPENGL_RENDERER_FACTORY_RENDERER_FACTORY_OPENGL_H_

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/renderer_factory.h"
#include "renderer/forwarding.h"

namespace ark {
namespace opengl {

class RendererFactoryOpenGL : public RendererFactory {
public:
    RendererFactoryOpenGL(const sp<Recycler>& recycler);

    virtual sp<RenderEngineContext> initialize(Ark::RendererVersion version) override;

    virtual void onSurfaceCreated(RenderEngineContext& glContext) override;

    virtual sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) override;
    virtual sp<Camera::Delegate> createCamera(Ark::RendererCoordinateSystem cs) override;
    virtual sp<Framebuffer> createFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask) override;
    virtual sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) override;
    virtual sp<PipelineFactory> createPipelineFactory() override;
    virtual sp<Texture::Delegate> createTexture(const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader) override;

private:
    void setVersion(Ark::RendererVersion version, RenderEngineContext& glContext);

private:
    sp<Recycler> _recycler;

};

}
}

#endif
