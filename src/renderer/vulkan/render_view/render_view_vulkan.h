#ifndef ARK_RENDERER_IMPL_RENDER_VIEW_RENDER_VIEW_OPENGL_H_
#define ARK_RENDERER_IMPL_RENDER_VIEW_RENDER_VIEW_OPENGL_H_

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"
#include "graphics/forwarding.h"
#include "graphics/inf/render_view.h"

#include "renderer/forwarding.h"

namespace ark {
namespace vulkan {

class VulkanAPI;

class RenderViewVulkan : public RenderView {
public:
    RenderViewVulkan(const sp<VulkanAPI>& vulkanApi, const sp<GLContext>& glContext, const sp<GLResourceManager>& glResources, const Viewport& viewport);

    virtual void onSurfaceCreated() override;
    virtual void onSurfaceChanged(uint32_t width, uint32_t height) override;
    virtual void onRenderFrame(const Color& backgroundColor, const sp<RenderCommand>& renderCommand) override;

private:
    void initialize(uint32_t width, uint32_t height);
    
private:
    sp<VulkanAPI> _vulkan_api;

    op<GraphicsContext> _graphics_context;
    Viewport _viewport;

};

}
}

#endif