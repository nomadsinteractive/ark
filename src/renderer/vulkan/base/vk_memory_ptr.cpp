#include "renderer/vulkan/base/vk_memory_ptr.h"

#include "core/inf/input.h"
#include "core/impl/writable/writable_memory.h"

#include "renderer/vulkan/base/vk_memory.h"

namespace ark {
namespace vulkan {

VKMemoryPtr::VKMemoryPtr(std::nullptr_t)
{
}

VKMemoryPtr::VKMemoryPtr(const VKMemory& memory, VkDeviceSize offset, VkDeviceSize size)
    : _stub(sp<Stub>::make(memory, offset, size))
{
}

VKMemoryPtr::operator bool() const
{
    return static_cast<bool>(_stub);
}

VkDeviceSize VKMemoryPtr::offset() const
{
    return _stub->_offset;
}

VkDeviceSize VKMemoryPtr::size() const
{
    return _stub->_size;
}

VkDeviceMemory VKMemoryPtr::vkMemory() const
{
    return _stub->_memory.vkMemory();
}

bool VKMemoryPtr::operator >(const VKMemoryPtr& other) const
{
    if(vkMemory() != other.vkMemory())
        return false;
    return _stub->_offset > other.offset();
}

bool VKMemoryPtr::operator <(const VKMemoryPtr& other) const
{
    if(vkMemory() != other.vkMemory())
        return false;
    return _stub->_offset < other.offset();
}

bool VKMemoryPtr::operator >=(const VKMemoryPtr& other) const
{
    if(vkMemory() != other.vkMemory())
        return false;
    return _stub->_offset >= other.offset();
}

bool VKMemoryPtr::operator <=(const VKMemoryPtr& other) const
{
    if(vkMemory() != other.vkMemory())
        return false;
    return _stub->_offset <= other.offset();
}

VkDeviceSize VKMemoryPtr::operator -(const VKMemoryPtr& other) const
{
    DCHECK(vkMemory() == other.vkMemory(), "Operation between two different memories");
    DCHECK(_stub->_offset >= other.offset(), "The other ptr is greater than myself");
    return _stub->_offset - other.offset();
}

VKMemoryPtr VKMemoryPtr::operator +(VkDeviceSize offset) const
{
    return VKMemoryPtr(_stub->_memory, _stub->_offset + offset, 0);
}

VKMemoryPtr::Stub* VKMemoryPtr::operator ->() const
{
    return _stub.get();
}

VKMemoryPtr::Stub::Stub(const VKMemory& memory, VkDeviceSize offset, VkDeviceSize size)
    : _memory(memory), _offset(offset), _size(size), _type_index(0)
{
}

void* VKMemoryPtr::Stub::map()
{
    return _memory.map(_offset, _size);
}

void VKMemoryPtr::Stub::unmap()
{
    _memory.unmap();
}

void VKMemoryPtr::Stub::upload(Input& input)
{
    CHECK(input.size() <= _size, "Uploader size: %d, memory size: %d", input.size(), _size);

    WritableMemory writer(map());
    input.upload(writer);

    unmap();
}

}
}
