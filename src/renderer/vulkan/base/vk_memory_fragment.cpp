#include "renderer/vulkan/base/vk_memory_fragment.h"

#include "renderer/inf/uploader.h"


#include "renderer/vulkan/base/vk_memory.h"

namespace ark {
namespace vulkan {

VKMemoryFragment::VKMemoryFragment(const sp<VKMemory>& memory, VkDeviceSize offset, VkDeviceSize size)
    : _memory(memory), _offset(offset), _size(size)
{
}

VkDeviceSize VKMemoryFragment::offset() const
{
    return _offset;
}

VkDeviceSize VKMemoryFragment::size() const
{
    return _size;
}

void* VKMemoryFragment::map()
{
    return _memory->map(_offset, _size);
}

void VKMemoryFragment::unmap()
{
    _memory->unmap();
}

void VKMemoryFragment::upload(Uploader& uploader)
{
    DCHECK(uploader.size() <= _size, "Uploader size: %d, memory size: %d", uploader.size(), _size);

    uint8_t* buf = reinterpret_cast<uint8_t*>(map());
    uploader.upload([&buf](void* data, size_t size) {
        memcpy(buf, data, size);
        buf += size;
    });

    unmap();
}

}
}
