#include "renderer/impl/render_view_factory/render_view_factory_opengl.h"

#include "renderer/impl/render_view/render_view_opengl.h"

namespace ark {

RenderViewFactoryOpenGL::RenderViewFactoryOpenGL(const sp<GLResourceManager>& glResources)
    : _resource_manager(glResources)
{
}

sp<RenderView> RenderViewFactoryOpenGL::createRenderView(const sp<GLContext>& glContext, const Viewport& viewport)
{
    return sp<RenderView>::adopt(new RenderViewOpenGL(glContext, _resource_manager, viewport));
}

}
