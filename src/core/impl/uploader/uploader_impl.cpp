#include "core/impl/uploader/uploader_impl.h"

#include "core/impl/writable/writable_with_offset.h"
#include "core/util/log.h"

namespace ark {

UploaderImpl::UploaderImpl(const size_t size)
    : Uploader(size)
{
}

UploaderImpl::UploaderImpl(const Map<size_t, sp<Uploader>>& uploaderMap, const size_t size)
    : Uploader(size)
{
    for(const auto& [k, v] : uploaderMap)
        put(k, v);

    if(_size == 0)
        _size = calculateUploaderSize();
    else
        CHECK(calculateUploaderSize() <= _size, "Uploader size overflow, size %zd is not long enough to fit all the inputs", _size);
}

bool UploaderImpl::update(const uint64_t timestamp)
{
    bool dirty = false;
    for(UploaderStub& i : _uploaders)
    {
        i._dirty_updated = i._uploader->update(timestamp);
        dirty = dirty || i._dirty_updated || i._dirty_marked;
    }
    return dirty;
}

void UploaderImpl::upload(Writable& writable)
{
    THREAD_CHECK(THREAD_NAME_ID_CORE);
    for(UploaderStub& i : _uploaders)
        if(i._dirty_updated || i._dirty_marked)
        {
            WritableWithOffset wwo(writable, i._offset);
            i._uploader->upload(wwo);
            i._dirty_marked = false;
        }
}

void UploaderImpl::put(const size_t offset, sp<Uploader> uploader)
{
    CHECK(_size >= offset + uploader->size(), "Uploader size overflow, size(%zd) is not big enough to fit this one(offset: %zd, size: %zd)", _size, offset, uploader->size());
    sp<Boolean::Impl>& discarded = _uploader_states[offset];
    if(discarded != nullptr)
    {
        remove(offset);
        return put(offset, std::move(uploader));
    }
    if(!discarded)
        discarded = sp<Boolean::Impl>::make(false);
    _uploaders.emplace_back({offset, std::move(uploader), discarded});
}

void UploaderImpl::remove(const size_t offset)
{
    const auto iter = _uploader_states.find(offset);
    CHECK(iter != _uploader_states.end(), "Uploader offset(%zd) unoccupied", offset);
    iter->second->set(true);
    _uploader_states.erase(iter);
}

void UploaderImpl::reset(sp<Uploader> uploader)
{
    _size = uploader->size();
    for(auto& [i, j] : _uploader_states)
        j->set(true);
    _uploader_states.clear();
    put(0, std::move(uploader));
}

void UploaderImpl::markDirty()
{
    for(UploaderStub& i : _uploaders)
        i._dirty_marked = true;
}

size_t UploaderImpl::calculateUploaderSize()
{
    size_t size = 0;
    for(const UploaderStub& i : _uploaders)
        size = std::max(i._offset + i._uploader->size(), size);
    return size;
}

}
