#include "renderer/impl/uploader/uploader_impl.h"

#include "core/inf/input.h"
#include "core/impl/updatable/updatable_wrapper.h"
#include "core/impl/updatable/updatable_composite.h"

namespace ark {

UploaderImpl::UploaderImpl(const std::map<size_t, sp<Input>>& inputs, size_t size)
    : Uploader(size), _inputs(makeInputs(inputs)), _updatable(sp<UpdatableWrapper>::make(makeUpdatable()))
{
    if(_size == 0)
        _size = calculateUploaderSize();
    else
        CHECK(calculateUploaderSize() <= _size, "Uploader size overflow, size %zd is not long enough to fill all the inputs", _size);
}

size_t UploaderImpl::size()
{
    return _size;
}

void UploaderImpl::upload(Writable& writable)
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    for(const auto& [i, j] : _inputs)
        j->upload(writable, _buf, i);
}

sp<Updatable> UploaderImpl::updatable()
{
    return _updatable;
}

void UploaderImpl::addInput(size_t offset, sp<Input> input)
{
    _inputs[offset] = sp<InputImpl>::make(std::move(input));
    _updatable->reset(makeUpdatable());
}

void UploaderImpl::removeInput(size_t offset)
{
    const auto iter = _inputs.find(offset);
    CHECK(iter != _inputs.end(), "Input with offset(%zd) does not exist", offset);
    _inputs.erase(iter);
    _updatable->reset(makeUpdatable());
}

sp<Updatable> UploaderImpl::makeUpdatable() const
{
    std::vector<sp<Updatable>> updatables;
    for(const auto& i : _inputs)
        updatables.push_back(i.second);
    return sp<UpdatableComposite>::make(std::move(updatables));
}

std::map<size_t, sp<UploaderImpl::InputImpl>> UploaderImpl::makeInputs(const std::map<size_t, sp<Input>>& inputs) const
{
    std::map<size_t, sp<InputImpl>> res;
    for(const auto& [i, j] : inputs)
        res.insert(std::make_pair(i, sp<InputImpl>::make(j)));
    return res;
}

size_t UploaderImpl::calculateUploaderSize() const
{
    size_t size = 0;
    for(const auto& [i, j] : _inputs)
    {
        size_t maxSize = i + j->_input->size();
        if(maxSize > size)
            size = maxSize;
    }
    return size;
}

UploaderImpl::InputImpl::InputImpl(sp<Input> input)
    : _input(std::move(input)), _dirty(true)
{
}

bool UploaderImpl::InputImpl::update(uint64_t timestamp)
{
    _dirty = _input->update(timestamp) || _dirty;
    return _dirty;
}

void UploaderImpl::InputImpl::upload(Writable& writable, std::vector<uint8_t>& buf, size_t offset)
{
    if(_dirty)
    {
        uint32_t size = _input->size();
        if(buf.size() < size)
            buf.resize(size);
        _input->flat(buf.data());
        writable.write(buf.data(), size, offset);
        _dirty = false;
    }
}

}
