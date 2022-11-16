#ifndef ARK_RENDERER_VULKAN_BASE_VK_TEXTURE_H_
#define ARK_RENDERER_VULKAN_BASE_VK_TEXTURE_H_

#include "core/base/notifier.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKTexture : public Texture::Delegate {
public:
    VKTexture(sp<Recycler> recycler, sp<VKRenderer> renderer, uint32_t width, uint32_t height, sp<Texture::Parameters> parameters);
    ~VKTexture() override;

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader) override;
    virtual ResourceRecycleFunc recycle() override;

    virtual void clear(GraphicsContext& graphicsContext) override;
    virtual bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;
    virtual void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata) override;

    const VkDescriptorImageInfo& vkDescriptor() const;

protected:
    void doUploadBitmap(const Bitmap& bitmap, size_t imageDataSize, const std::vector<bytearray>& imagedata);

private:
    ResourceRecycleFunc doRecycle();

    void copyBitmap(uint8_t* buf, const Bitmap& bitmap, const bytearray& imagedata, size_t imageDataSize);

private:
    sp<Recycler> _recycler;
    sp<VKRenderer> _renderer;
    uint32_t _width, _height;
    sp<Texture::Parameters> _parameters;
    uint32_t _num_faces;

    VkImage _image;
    VkDeviceMemory _memory;
    uint32_t _mip_levels;
    VkDescriptorImageInfo _descriptor;

};


}
}

#endif
