#include "core/impl/integer/integer_by_array.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"

namespace ark {

IntegerByArray::IntegerByArray(const sp<IntArray>& array, IntegerType::Repeat repeat, Notifier notifier)
    : Delegate(array), _repeat(repeat), _position(0), _step(1), _notifier(std::move(notifier))
{
    DCHECK(array && array->length() > 0, "Empty array");
    DCHECK(repeat != IntegerType::REPEAT_REVERSE || array->length() > 1, "A reversable array must have at least 2 elements");
}

int32_t IntegerByArray::val()
{
    int32_t v = _delegate->buf()[_position];
    if(_step)
    {
        int32_t length = static_cast<int32_t>(_delegate->length());
        _position += _step;
        if(_position == length)
        {
            if(_repeat == IntegerType::REPEAT_NONE)
            {
                _step = 0;
                _position = length - 1;
                _notifier.notify();
                return -1;
            }
            if(_repeat == IntegerType::REPEAT_LAST)
            {
                _step = 0;
                _position = length - 1;
                _notifier.notify();
            }
            else if(_repeat == IntegerType::REPEAT_RESTART)
            {
                _position = 0;
                _notifier.notify();
            }
            else if(_repeat == IntegerType::REPEAT_REVERSE || _repeat == IntegerType::REPEAT_REVERSE_RESTART)
            {
                _position = length - 2;
                _step = -1;
            }
        }
        else if(_position == -1)
        {
            if(_repeat == IntegerType::REPEAT_REVERSE)
            {
                _position = 0;
                _step = 0;
            }
            else
            {
                _position = 1;
                _step = 1;
            }
            _notifier.notify();
        }
    }
    return v;
}

bool IntegerByArray::update(uint64_t /*timestamp*/)
{
    return true;
}

}
