#ifndef ARK_RENDERER_VULKAN_BASE_VK_FRAMEBUFFER_H_
#define ARK_RENDERER_VULKAN_BASE_VK_FRAMEBUFFER_H_

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {
namespace vulkan {

class VKFramebuffer : public Resource {
public:
    VKFramebuffer(const sp<Recycler>& recycler, const sp<Texture>& texture);
    ~VKFramebuffer() override;

    virtual uint64_t id() override;

    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

    const sp<Texture>& texture() const;

private:
    sp<Recycler> _recycler;
    sp<Texture> _texture;
};

}
}

#endif
