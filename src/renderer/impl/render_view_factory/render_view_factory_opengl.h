#ifndef ARK_RENDERER_IMPL_RENDER_VIEW_FACTORY_RENDER_VIEW_FACTORY_OPENGL_H_
#define ARK_RENDERER_IMPL_RENDER_VIEW_FACTORY_RENDER_VIEW_FACTORY_OPENGL_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/render_view_factory.h"
#include "renderer/forwarding.h"

namespace ark {

class RenderViewFactoryOpenGL : public RenderViewFactory {
public:
    RenderViewFactoryOpenGL(const sp<GLResourceManager>& glResources);

    virtual sp<RenderView> createRenderView(const sp<GLContext>& glContext, const Viewport& viewport) override;

private:
    sp<GLResourceManager> _resource_manager;
};

}

#endif
