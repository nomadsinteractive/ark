#include "core/impl/uploader/uploader_impl.h"

#include "core/impl/writable/writable_with_offset.h"
#include "core/util/log.h"

namespace ark {

UploaderImpl::UploaderImpl(size_t size)
    : Uploader(size)
{
}

UploaderImpl::UploaderImpl(const std::map<size_t, sp<Uploader>>& inputMap, size_t size)
    : Uploader(size)
{
    for(const auto& [i, j] : inputMap)
        addInput(i, j);

    if(_size == 0)
        _size = calculateUploaderSize();
    else
        CHECK(calculateUploaderSize() <= _size, "Input size overflow, size %zd is not long enough to fit all the inputs", _size);
}

bool UploaderImpl::update(uint64_t timestamp)
{
    bool dirty = false;
    for(UploaderStub& i : _uploaders)
    {
        i._dirty_updated = i._input->update(timestamp);
        dirty = dirty || i._dirty_updated || i._dirty_marked;
    }
    return dirty;
}

void UploaderImpl::upload(Writable& writable)
{
    THREAD_CHECK(THREAD_ID_CORE);
    for(UploaderStub& i : _uploaders)
        if(i._dirty_updated || i._dirty_marked)
        {
            WritableWithOffset wwo(writable, i._offset);
            i._input->upload(wwo);
            i._dirty_marked = false;
        }
}

void UploaderImpl::addInput(size_t offset, sp<Uploader> input)
{
    LOGD("[%p] offset: %zd, size: %zd", this, offset, input->size());
    sp<Boolean::Impl>& disposed = _uploader_states[offset];
    CHECK(disposed == nullptr, "Input offset(%zd) was occupied already", offset);
    CHECK(_size >= offset + input->size(), "Input size overflow, size(%zd) is not big enough to fit this one(offset: %zd, size: %zd)", _size, offset, input->size());
    if(!disposed)
        disposed = sp<Boolean::Impl>::make(false);
    _uploaders.emplace_back(UploaderStub(offset, std::move(input), disposed));
}

void UploaderImpl::removeInput(size_t offset)
{
    LOGD("[%p] offset: %zd", this, offset);
    const auto iter = _uploader_states.find(offset);
    CHECK(iter != _uploader_states.end(), "Input offset(%zd) unoccupied", offset);
    iter->second->set(true);
    _uploader_states.erase(iter);
}

void UploaderImpl::reset(sp<Uploader> uploader)
{
    _size = uploader->size();
    for(auto& [i, j] : _uploader_states)
        j->set(true);
    _uploader_states.clear();
    addInput(0, std::move(uploader));
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
    {
        size_t maxSize = i._offset + i._input->size();
        if(maxSize > size)
            size = maxSize;
    }
    return size;
}

UploaderImpl::UploaderStub::UploaderStub(size_t offset, sp<Uploader> input, sp<Boolean> discarded)
    : _offset(offset), _input(std::move(input)), _dirty_updated(true), _dirty_marked(true), _discarded(discarded)
{
    DASSERT(_input);
}

bool UploaderImpl::UploaderStub::isDiscarded() const
{
    return _discarded->val();
}

}
