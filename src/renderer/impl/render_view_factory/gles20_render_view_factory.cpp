#include "renderer/impl/render_view_factory/gles20_render_view_factory.h"

#include "renderer/impl/render_view/gles20_render_view.h"

namespace ark {

GLES20RenderViewFactory::GLES20RenderViewFactory(const sp<GLResourceManager>& glResources)
    : _resource_manager(glResources)
{
}

sp<RenderView> GLES20RenderViewFactory::createRenderView(const sp<GLContext>& glContext, const Viewport& viewport)
{
    return sp<RenderView>::adopt(new GLES20RenderView(glContext, _resource_manager, viewport));
}

}
