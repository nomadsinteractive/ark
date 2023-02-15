#include "core/impl/input/input_impl.h"

#include <algorithm>

#include "core/impl/writable/writable_with_offset.h"

namespace ark {

InputImpl::InputImpl(size_t size)
    : Input(size)
{
}

InputImpl::InputImpl(const std::map<size_t, sp<Input>>& inputs, size_t size)
    : Input(size), _inputs(makeInputs(inputs))
{
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

void InputImpl::addInput(size_t offset, sp<Input> input)
{
    CHECK(_size >= offset + input->size(), "Input size overflow, size(%zd) is not big enough to fit this one(offset: %zd, size: %zd)", _size, offset, input->size());
    _inputs.insert(std::upper_bound(_inputs.begin(), _inputs.end(), offset, _input_stub_comp), InputStub(offset, std::move(input)));
}

void InputImpl::removeInput(size_t offset)
{
    for(auto iter = _inputs.begin(); iter != _inputs.end(); )
    {
        if(iter->_offset == offset)
            iter = _inputs.erase(iter);
        else
            ++iter;
    }
}

void InputImpl::markDirty()
{
    for(InputStub& i : _inputs)
        i._dirty_marked = true;
}

std::vector<InputImpl::InputStub> InputImpl::makeInputs(const std::map<size_t, sp<Input>>& inputs) const
{
    std::vector<InputImpl::InputStub> res;
    for(const auto& [i, j] : inputs)
        res.insert(std::upper_bound(res.begin(), res.end(), i, _input_stub_comp), InputStub(i, j));
    return res;
}

size_t InputImpl::calculateUploaderSize() const
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

bool InputImpl::_input_stub_comp(size_t offset, const InputStub& inputStub)
{
    return offset < inputStub._offset;
}

InputImpl::InputStub::InputStub(size_t offset, sp<Input> input)
    : _offset(offset), _input(std::move(input)), _dirty_updated(true), _dirty_marked(true)
{
}

}
