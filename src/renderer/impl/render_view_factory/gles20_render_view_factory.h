#ifndef ARK_RENDERER_GLES20_IMPL_RENDER_VIEW_FACTORY_GLES20_RENDER_VIEW_FACTORY_H_
#define ARK_RENDERER_GLES20_IMPL_RENDER_VIEW_FACTORY_GLES20_RENDER_VIEW_FACTORY_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/render_view_factory.h"
#include "renderer/forwarding.h"

namespace ark {

class GLES20RenderViewFactory : public RenderViewFactory {
public:
    GLES20RenderViewFactory(const sp<GLResourceManager>& glResources);

    virtual sp<RenderView> createRenderView(const sp<GLContext>& glContext, const Viewport& viewport) override;

private:
    sp<GLResourceManager> _resource_manager;
};

}

#endif
