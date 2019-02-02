#include "renderer/vulkan/base/vk_framebuffer.h"

#include "renderer/base/recycler.h"
#include "renderer/base/texture.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

VKFramebuffer::VKFramebuffer(const sp<Recycler>& recycler, const sp<Texture>& texture)
    : _recycler(recycler), _texture(texture)
{
}

VKFramebuffer::~VKFramebuffer()
{
}

uint64_t VKFramebuffer::id()
{
    return 0;
}

void VKFramebuffer::upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader)
{
}

Resource::RecycleFunc VKFramebuffer::recycle()
{
    return [](GraphicsContext&) {
    };
}

const sp<Texture>& VKFramebuffer::texture() const
{
    return _texture;
}

}
}
