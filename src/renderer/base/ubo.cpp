#include "renderer/base/ubo.h"

#include "core/base/memory_pool.h"
#include "core/impl/array/dynamic_array.h"
#include "core/epi/changed.h"
#include "core/inf/flatable.h"

namespace ark {

UBO::UBO(std::vector<Uniform> uniforms)
    : _uniforms(std::move(uniforms)), _dirty_flags(sp<DynamicArray<uint8_t>>::make(_uniforms.size()))
{
    size_t size = 0;
    for(const auto& i : _uniforms)
    {
        size_t s = i.flatable()->size();
        _slots.push_back(std::make_pair(size, s));
        size += s;
        DCHECK(size % 4 == 0, "Uniform aligment error, offset: %d", size);
    }
    _buffer = sp<DynamicArray<uint8_t>>::make(size);
    memset(_buffer->buf(), 0, _buffer->size());

    doSnapshot();
}

bool UBO::doSnapshot() const
{
    size_t offset = 0;
    uint8_t* dirtyFlags = _dirty_flags->buf();
    bool dirty = false;
    for(size_t i = 0; i < _uniforms.size(); ++i)
    {
        const Uniform& uniform = _uniforms.at(i);
        const sp<Flatable>& flatable = uniform.flatable();
        dirtyFlags[i] = static_cast<uint8_t>(uniform.dirty());
        dirty = dirty || dirtyFlags[i];
        if(dirtyFlags[i])
            flatable->flat(_buffer->buf() + offset);
        offset += flatable->size();
    }
    return dirty;
}

Layer::UBOSnapshot UBO::snapshot(MemoryPool& memoryPool) const
{
    Layer::UBOSnapshot ubo;
    bool dirty = doSnapshot();
    if(dirty)
    {
        ubo._dirty_flags = memoryPool.allocate(_dirty_flags->size());
        memcpy(ubo._dirty_flags->buf(), _dirty_flags->buf(), _dirty_flags->size());
        ubo._buffer = memoryPool.allocate(_buffer->size());
        memcpy(ubo._buffer->buf(), _buffer->buf(), _buffer->size());
    }
    return ubo;
}

const std::vector<Uniform>& UBO::uniforms() const
{
    return _uniforms;
}

}
