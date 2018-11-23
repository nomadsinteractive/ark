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
    RendererFactoryOpenGL(const sp<GLResourceManager>& glResources);

    virtual void initialize(GLContext& glContext) override;

    virtual void setGLVersion(Ark::GLVersion version, GLContext& glContext) override;

    virtual sp<RenderView> createRenderView(const sp<GLContext>& glContext, const Viewport& viewport) override;

    virtual sp<PipelineFactory> createPipelineFactory() override;

private:
    sp<GLResourceManager> _resource_manager;

};

}
}

#endif
