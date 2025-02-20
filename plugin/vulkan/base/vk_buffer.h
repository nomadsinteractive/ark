#pragma once

#include "core/base/observer.h"
#include "core/types/shared_ptr.h"

#include "renderer/base/buffer.h"

#include "vulkan/base/vk_device.h"
#include "vulkan/base/vk_memory_ptr.h"
#include "vulkan/util/vulkan_buffer.hpp"

namespace ark::plugin::vulkan {

class VKBuffer final : public Buffer::Delegate {
public:
    VKBuffer(sp<VKRenderer> renderer, sp<Recycler> recycler, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
    ~VKBuffer() override;
    DISALLOW_COPY_AND_ASSIGN(VKBuffer);

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext) override;
    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override;
    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override;
    ResourceRecycleFunc recycle() override;

    void reload(GraphicsContext& graphicsContext, const ByteArray::Borrowed& buf) const;

    const VkBuffer& vkBuffer() const;

    VkResult invalidate();

    const VkDescriptorBufferInfo& vkDescriptor() const;

    Observer& observer();

private:
    void allocateMemory(GraphicsContext& graphicsContext, const VkMemoryRequirements& memReqs);
    void ensureSize(GraphicsContext& graphicsContext, size_t size);

    void bind();
    VkResult flush() const;

    bool isDeviceLocal() const;
    bool isHostCoherent() const;
    bool isHostVisible() const;

private:
    sp<VKRenderer> _renderer;
    sp<Recycler> _recycler;

    /** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
    VkBufferUsageFlags _usage_flags;
    /** @brief Memory propertys flags to be filled by external source at buffer creation (to query at some later point) */
    VkMemoryPropertyFlags _memory_property_flags;

    VkDescriptorBufferInfo _descriptor;

    VKMemoryPtr _memory;
    VkMemoryRequirements _memory_requirements;

    Observer _observer;
};

}
