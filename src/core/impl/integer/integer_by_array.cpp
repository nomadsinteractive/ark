#include "core/impl/integer/integer_by_array.h"

#include "core/base/bean_factory.h"
#include "core/epi/disposable.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/documents.h"

namespace ark {

namespace {

class IntegerByArrayExpired : public Boolean {
public:
    IntegerByArrayExpired(const sp<IntegerByArray::Stub>& stub)
        : _stub(stub) {
    }

    virtual bool val() override {
        return _stub->_step == 0;
    }

private:
    sp<IntegerByArray::Stub> _stub;
};

}

IntegerByArray::IntegerByArray(const sp<IntArray>& array, IntegerByArray::Repeat repeat)
    : _stub(sp<Stub>::make(array, repeat))
{
    DCHECK(array && array->length() > 0, "Empty array");
    DCHECK(repeat != REPEAT_REVERSE || array->length() > 1, "A reversable array must have at least 2 elements");
}

int32_t IntegerByArray::val()
{
    return _stub->val();
}

IntegerByArray::Stub::Stub(const sp<IntArray>& array, IntegerByArray::Repeat repeat)
    : _array(array), _position(0), _repeat(repeat), _step(1)
{
}

int32_t IntegerByArray::Stub::val()
{
    int32_t v = _array->buf()[_position];
    if(_step)
    {
        _position += _step;
        if(_position == _array->length())
        {
            if(_repeat == IntegerByArray::REPEAT_NONE)
            {
                _position = _array->length() - 1;
                _step = 0;
            }
            else if(_repeat == IntegerByArray::REPEAT_RESTART)
                _position = 0;
            else if(_repeat == IntegerByArray::REPEAT_REVERSE)
            {
                _position = _array->length() - 2;
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

IntegerByArray::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _array(factory.ensureBuilder<IntArray>(manifest, "array"))
{
    const String& repeat = Documents::getAttribute(manifest, "repeat");
    if(repeat == "restart")
        _repeat = IntegerByArray::REPEAT_RESTART;
    else if(repeat == "reverse")
        _repeat = IntegerByArray::REPEAT_REVERSE;
    else
        _repeat = IntegerByArray::REPEAT_NONE;
}

sp<Integer> IntegerByArray::BUILDER::build(const sp<Scope>& args)
{
    sp<IntegerByArray> s = sp<IntegerByArray>::make(_array->build(args), _repeat);
    if(_repeat == IntegerByArray::REPEAT_NONE)
        s.absorb<Disposable>(sp<Disposable>::make(sp<IntegerByArrayExpired>::make(s->_stub)));
    return s;
}

}
