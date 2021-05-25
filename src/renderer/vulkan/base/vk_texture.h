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
    VKTexture(const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, uint32_t width, uint32_t height, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader);
    ~VKTexture() override;

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

    virtual bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;
    virtual void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata) override;

    const VkDescriptorImageInfo& vkDescriptor() const;

protected:
    void doUploadBitmap(const Bitmap& bitmap, size_t imageDataSize, const std::vector<bytearray>& imagedata);

private:
    void copyBitmap(uint8_t* buf, const Bitmap& bitmap, const bytearray& imagedata, size_t imageDataSize);

private:
    sp<Recycler> _recycler;
    sp<VKRenderer> _renderer;
    uint32_t _width, _height;
    sp<Texture::Parameters> _parameters;
    sp<Texture::Uploader> _uploader;
    uint32_t _num_faces;

    VkImage _image;
    VkDeviceMemory _memory;
    uint32_t _mip_levels;
    VkDescriptorImageInfo _descriptor;

};


}
}

#endif
