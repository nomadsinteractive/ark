#include "core/impl/integer/integer_wrapper.h"

#include "core/ark.h"

#include "app/base/application_context.h"

namespace ark {

IntegerWrapper::IntegerWrapper(const sp<Integer>& delegate)
    : _delegate(delegate), _value(0)
{
}

IntegerWrapper::IntegerWrapper(int32_t value)
    : _value(value)
{
}

int32_t IntegerWrapper::val()
{
    return _delegate ? _delegate->val() : _value;
}

const sp<Integer>& IntegerWrapper::delegate() const
{
    return _delegate;
}

void IntegerWrapper::setDelegate(const sp<Integer>& delegate)
{
    NOT_NULL(delegate);
    deferedUnref();
    _delegate = delegate;
}

void IntegerWrapper::set(int32_t value)
{
    deferedUnref();
    _value = value;
}

void IntegerWrapper::set(const sp<Integer>& delegate)
{
    deferedUnref();
    _delegate = delegate;
}

void IntegerWrapper::fix()
{
    if(_delegate)
    {
        _value = _delegate->val();
        deferedUnref();
    }
}

void IntegerWrapper::deferedUnref()
{
    if(_delegate)
        Ark::instance().applicationContext()->deferUnref(std::move(_delegate));
}

}
