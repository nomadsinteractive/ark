#include "core/impl/integer/integer_by_array.h"

#include "core/base/bean_factory.h"
#include "core/epi/disposed.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/documents.h"

namespace ark {

IntegerByArray::IntegerByArray(const sp<IntArray>& array, IntegerUtil::Repeat repeat)
    : _stub(sp<Stub>::make(array, repeat))
{
    DCHECK(array && array->length() > 0, "Empty array");
    DCHECK(repeat != IntegerUtil::REPEAT_REVERSE || array->length() > 1, "A reversable array must have at least 2 elements");
}

int32_t IntegerByArray::val()
{
    return _stub->val();
}

IntegerByArray::Stub::Stub(const sp<IntArray>& array, IntegerUtil::Repeat repeat)
    : _array(array), _position(0), _repeat(repeat), _step(1)
{
}

int32_t IntegerByArray::Stub::val()
{
    int32_t v = _array->buf()[_position];
    if(_step)
    {
        int32_t length = static_cast<int32_t>(_array->length());
        _position += _step;
        if(_position == length)
        {
            if(_repeat == IntegerUtil::REPEAT_NONE)
            {
                _step = 0;
                return -1;
            }
            else if(_repeat == IntegerUtil::REPEAT_RESTART)
                _position = 0;
            else if(_repeat == IntegerUtil::REPEAT_REVERSE)
            {
                _position = length - 2;
                _step = -1;
            }
        }
        else if(_position == -1)
        {
            _position = 1;
            _step = 1;
        }
    }
    return v;
}

}
