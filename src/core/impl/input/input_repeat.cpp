#include "core/impl/input/input_repeat.h"

#include "core/impl/writable/writable_with_offset.h"

namespace ark {

InputRepeat::InputRepeat(sp<Input> delegate, size_t length, size_t stride)
    : Input(length * (stride ? stride : delegate->size())), _delegate(std::move(delegate)), _length(length), _stride(stride ? stride : _delegate->size())
{
    ASSERT(_stride >= _delegate->size());
}

bool InputRepeat::update(uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

void InputRepeat::upload(Writable& writable)
{
    WritableWithOffset w(writable, 0);
    for(size_t i = 0; i < _length; ++i)
    {
        w.setOffset(i * _stride);
        _delegate->upload(w);
    }
}

}
