#ifndef ARK_RENDERER_IMPL_RENDER_VIEW_RENDER_VIEW_OPENGL_H_
#define ARK_RENDERER_IMPL_RENDER_VIEW_RENDER_VIEW_OPENGL_H_

#include <vector>

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/color.h"
#include "renderer/base/graphics_context.h"
#include "graphics/base/viewport.h"
#include "graphics/forwarding.h"
#include "graphics/inf/render_view.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class VKUtil;

class RenderViewVulkan : public RenderView {
public:
    RenderViewVulkan(const sp<VKRenderer>& renderer, const sp<RenderContext>& renderContext, const sp<RenderController>& renderController);

    virtual void onSurfaceCreated() override;
    virtual void onSurfaceChanged(uint32_t width, uint32_t height) override;
    virtual void onRenderFrame(const Color& backgroundColor, const sp<RenderCommand>& renderCommand) override;

private:
    void makeCommandBuffers(GraphicsContext& graphicsContext, const Color& backgroundColor);
    
private:
    sp<VKRenderer> _renderer;
    sp<VKGraphicsContext> _vk_context;

    op<GraphicsContext> _graphics_context;
};

}
}

#endif
