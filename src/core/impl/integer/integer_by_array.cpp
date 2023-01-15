#include "core/impl/integer/integer_by_array.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"

namespace ark {

IntegerByArray::IntegerByArray(sp<IntArray> array, IntegerType::Repeat repeat, Notifier notifier)
    : Wrapper(std::move(array)), _repeat(repeat), _position(0), _step(1), _notifier(std::move(notifier))
{
    CHECK(_wrapped && _wrapped->length() > 0, "Empty array");
    CHECK(repeat != IntegerType::REPEAT_REVERSE || _wrapped->length() > 1, "A reversable array must have at least 2 elements");
}

int32_t IntegerByArray::val()
{
    if(_position < 0)
        return -1;

    int32_t length = static_cast<int32_t>(_wrapped->length());

    if(_position >= length)
        return -1;

    int32_t v = _wrapped->buf()[_position];
    if(_step)
    {
        _position += _step;
        if(_position == length)
        {
            IntegerType::Repeat action = static_cast<IntegerType::Repeat>(_repeat & IntegerType::REPEAT_ACTION_MASK);
            if(action == IntegerType::REPEAT_REVERSE)
            {
                _position = length - 2;
                _step = -1;
            }
            else
            {
                if(_repeat & IntegerType::REPEAT_LAST)
                {
                    _step = 0;
                    _position = length - 1;
                }
                else if(_repeat & IntegerType::REPEAT_LOOP)
                    _position = 0;

                if(_repeat & IntegerType::REPEAT_NOTIFY)
                    _notifier.notify();
            }
        }
        else if(_position == -1)
        {
            if(_repeat & IntegerType::REPEAT_LAST)
            {
                _position = 0;
                _step = 0;
            }
            else if(_repeat & IntegerType::REPEAT_LOOP)
            {
                _position = 1;
                _step = 1;
            }
            if(_repeat & IntegerType::REPEAT_NOTIFY)
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
