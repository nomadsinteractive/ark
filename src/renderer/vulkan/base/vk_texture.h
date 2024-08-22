#pragma once

#include "core/base/observer.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKTexture final : public Texture::Delegate {
public:
    VKTexture(sp<Recycler> recycler, sp<VKRenderer> renderer, uint32_t width, uint32_t height, sp<Texture::Parameters> parameters);
    ~VKTexture() override;

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader) override;
    ResourceRecycleFunc recycle() override;

    void clear(GraphicsContext& graphicsContext) override;
    bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;
    void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& images) override;

    const VkDescriptorImageInfo& vkDescriptor() const;
    Observer& observer();

protected:
    void doUploadBitmap(const Bitmap& bitmap, size_t imageDataSize, const std::vector<bytearray>& imagedata);

private:
    ResourceRecycleFunc doRecycle();

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

    Observer _observer;
};


}
