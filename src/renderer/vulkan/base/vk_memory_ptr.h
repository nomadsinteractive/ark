#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/base/vk_memory.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKMemoryPtr {
public:
    class Stub {
    public:
        Stub(const VKMemory& memory, VkDeviceSize offset, VkDeviceSize size);

        void* map();
        void unmap();

        void upload(Uploader& input);

    private:
        VKMemory _memory;

        VkDeviceSize _offset;
        VkDeviceSize _size;
        uint32_t _type_index;

        friend class VKHeap;
        friend class VKMemoryPtr;
    };

public:
    VKMemoryPtr() = default;
    VKMemoryPtr(std::nullptr_t);
    VKMemoryPtr(const VKMemory& memory, VkDeviceSize offset, VkDeviceSize size);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(VKMemoryPtr);

    explicit operator bool() const;

    VkDeviceSize offset() const;
    VkDeviceSize size() const;

    VkDeviceMemory vkMemory() const;

    bool operator >(const VKMemoryPtr& other) const;
    bool operator <(const VKMemoryPtr& other) const;
    bool operator >=(const VKMemoryPtr& other) const;
    bool operator <=(const VKMemoryPtr& other) const;

    VkDeviceSize operator -(const VKMemoryPtr& other) const;
    VKMemoryPtr operator +(VkDeviceSize offset) const;

    Stub* operator ->() const;

private:
    sp<Stub> _stub;

    friend class VKHeap;
};


}
}
