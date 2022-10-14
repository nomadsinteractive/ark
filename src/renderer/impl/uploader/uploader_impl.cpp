#include "renderer/impl/uploader/uploader_impl.h"

#include "core/inf/input.h"
#include "core/impl/updatable/updatable_wrapper.h"
#include "core/impl/updatable/updatable_composite.h"

namespace ark {

static size_t calculateUploaderSize(const std::map<size_t, sp<Input>>& inputs)
{
    size_t size = 0;
    for(const auto& [i, j] : inputs)
    {
        size_t maxSize = i + j->size();
        if(maxSize > size)
            size = maxSize;
    }
    return size;
}


UploaderImpl::UploaderImpl(std::map<size_t, sp<Input>> inputs, size_t size)
    : Uploader(size), _inputs(std::move(inputs)), _updatable(sp<UpdatableWrapper>::make(makeUpdatable()))
{
    if(_size == 0)
        _size = calculateUploaderSize(_inputs);
    else
        CHECK(calculateUploaderSize(_inputs) <= _size, "Uploader size overflow, size %zd is not long enough to fill all the inputs", _size);
}

size_t UploaderImpl::size()
{
    return _size;
}

void UploaderImpl::upload(Writable& writable)
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    for(const auto& [i, j] : _inputs)
    {
        auto inputSize = j->size();
        if(_buf.size() < inputSize)
            _buf.resize(inputSize);
        j->flat(_buf.data());
        writable.write(_buf.data(), inputSize, i);
    }
}

sp<Updatable> UploaderImpl::updatable()
{
    return _updatable;
}

void UploaderImpl::addInput(size_t offset, sp<Input> input)
{
    _inputs[offset] = std::move(input);
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

}
