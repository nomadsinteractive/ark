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

    virtual sp<RenderContext> initialize(Ark::RendererVersion version) override;

    virtual void onSurfaceCreated(RenderContext& glContext) override;

    virtual sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) override;
    virtual sp<Framebuffer> createFramebuffer(const sp<Renderer>& renderer, const sp<Texture>& texture) override;
    virtual sp<RenderView> createRenderView(const sp<RenderContext>& renderContext, const sp<RenderController>& renderController) override;
    virtual sp<PipelineFactory> createPipelineFactory() override;
    virtual sp<Texture> createTexture(uint32_t width, uint32_t height, const sp<Texture::Uploader>& uploader) override;

private:
    void setVersion(Ark::RendererVersion version, RenderContext& glContext);

private:
    sp<Recycler> _recycler;

};

}
}

#endif
