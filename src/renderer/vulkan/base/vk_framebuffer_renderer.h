#ifndef ARK_RENDERER_VULKAN_BASE_FRAMEBUFFER_RENDERER_H_
#define ARK_RENDERER_VULKAN_BASE_FRAMEBUFFER_RENDERER_H_

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class VKFramebufferRenderer : public Renderer {
public:
    VKFramebufferRenderer(const sp<Renderer>& delegate, const sp<VKFramebuffer>& framebuffer);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

private:
    sp<Renderer> _delegate;
    sp<VKFramebuffer> _fbo;
    sp<RenderCommand> _pre_draw;
    sp<RenderCommand> _post_draw;

};

}
}

#endif
