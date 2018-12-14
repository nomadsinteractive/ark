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
    RendererFactoryOpenGL(const sp<ResourceManager>& glResources);

    virtual void initialize(GLContext& glContext) override;
    virtual void setGLVersion(Ark::RendererVersion version, GLContext& glContext) override;

    virtual sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Uploader>& uploader) override;
    virtual sp<RenderView> createRenderView(const sp<GLContext>& glContext, const Viewport& viewport) override;
    virtual sp<PipelineFactory> createPipelineFactory() override;
    virtual sp<Texture> createTexture(const sp<Recycler>& recycler, uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmap) override;

private:
    sp<ResourceManager> _resource_manager;

};

}
}

#endif
