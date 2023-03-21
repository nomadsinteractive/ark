#include "core/impl/uploader/input_impl.h"

#include "core/impl/writable/writable_with_offset.h"
#include "core/util/log.h"

namespace ark {

InputImpl::InputImpl(size_t size)
    : Uploader(size)
{
}

InputImpl::InputImpl(const std::map<size_t, sp<Uploader>>& inputMap, size_t size)
    : Uploader(size)
{
    for(const auto& [i, j] : inputMap)
        addInput(i, j);

    if(_size == 0)
        _size = calculateUploaderSize();
    else
        CHECK(calculateUploaderSize() <= _size, "Input size overflow, size %zd is not long enough to fit all the inputs", _size);
}

bool InputImpl::update(uint64_t timestamp)
{
    bool dirty = false;
    for(InputStub& i : _inputs)
    {
        i._dirty_updated = i._input->update(timestamp);
        dirty = dirty || i._dirty_updated || i._dirty_marked;
    }
    return dirty;
}

void InputImpl::upload(Writable& writable)
{
    THREAD_CHECK(THREAD_ID_CORE);
    for(InputStub& i : _inputs)
        if(i._dirty_updated || i._dirty_marked)
        {
            WritableWithOffset wwo(writable, i._offset);
            i._input->upload(wwo);
            i._dirty_marked = false;
        }
}

void InputImpl::addInput(size_t offset, sp<Uploader> input)
{
    LOGD("[%p] offset: %zd, size: %zd", this, offset, input->size());
    sp<Boolean::Impl>& disposed = _inputs_disposed[offset];
    CHECK(disposed == nullptr, "Input offset(%zd) was occupied already", offset);
    CHECK(_size >= offset + input->size(), "Input size overflow, size(%zd) is not big enough to fit this one(offset: %zd, size: %zd)", _size, offset, input->size());
    if(!disposed)
        disposed = sp<Boolean::Impl>::make(false);
    _inputs.emplace_back(InputStub(offset, std::move(input), disposed));
}

void InputImpl::removeInput(size_t offset)
{
    LOGD("[%p] offset: %zd", this, offset);
    const auto iter = _inputs_disposed.find(offset);
    CHECK(iter != _inputs_disposed.end(), "Input offset(%zd) unoccupied", offset);
    iter->second->set(true);
    _inputs_disposed.erase(iter);
}

void InputImpl::markDirty()
{
    for(InputStub& i : _inputs)
        i._dirty_marked = true;
}

size_t InputImpl::calculateUploaderSize()
{
    size_t size = 0;
    for(const InputStub& i : _inputs)
    {
        size_t maxSize = i._offset + i._input->size();
        if(maxSize > size)
            size = maxSize;
    }
    return size;
}

InputImpl::InputStub::InputStub(size_t offset, sp<Uploader> input, sp<Boolean> disposed)
    : _offset(offset), _input(std::move(input)), _dirty_updated(true), _dirty_marked(true), _disposed(disposed)
{
    DASSERT(_input);
}

bool InputImpl::InputStub::isDisposed() const
{
    return _disposed->val();
}

}
