#ifndef ARK_RENDERER_VULKAN_BASE_VK_MEMORY_FRAGMENT_H_
#define ARK_RENDERER_VULKAN_BASE_VK_MEMORY_FRAGMENT_H_

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKMemoryFragment {
public:
    VKMemoryFragment(const sp<VKMemory>& memory, VkDeviceSize offset, VkDeviceSize size);

    VkDeviceSize offset() const;
    VkDeviceSize size() const;

    void* map();
    void unmap();

    void upload(Uploader& uploader);

private:
    sp<VKMemory> _memory;

    VkDeviceSize _offset;
    VkDeviceSize _size;
};


}
}

#endif
