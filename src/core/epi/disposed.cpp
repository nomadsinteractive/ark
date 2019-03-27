#include "core/epi/disposed.h"

#include "core/base/bean_factory.h"
#include "core/impl/boolean/boolean_and.h"
#include "core/impl/boolean/boolean_not.h"
#include "core/impl/boolean/boolean_or.h"
#include "core/impl/variable/variable_wrapper.h"

namespace ark {

Disposed::Disposed(bool disposed)
    : _disposed(sp<BooleanWrapper>::make(disposed))
{
}

Disposed::Disposed(const sp<Boolean>& disposed)
    : _disposed(sp<BooleanWrapper>::make(disposed))
{
}

bool Disposed::isDisposed() const
{
    return _disposed->val();
}

bool Disposed::__bool__(const sp<Disposed>& self)
{
    return self->isDisposed();
}

sp<Boolean> Disposed::__or__(const sp<Disposed>& lvalue, const sp<Disposed>& rvalue)
{
    return sp<BooleanOr>::make(lvalue->toBoolean(), rvalue->toBoolean());
}

sp<Boolean> Disposed::__or__(const sp<Disposed>& lvalue, const sp<Boolean>& rvalue)
{
    return sp<BooleanOr>::make(lvalue->toBoolean(), rvalue);
}

sp<Boolean> Disposed::__or__(const sp<Boolean>& lvalue, const sp<Disposed>& rvalue)
{
    return sp<BooleanOr>::make(lvalue, rvalue->toBoolean());
}

sp<Boolean> Disposed::__and__(const sp<Disposed>& lvalue, const sp<Disposed>& rvalue)
{
    return sp<BooleanAnd>::make(lvalue->toBoolean(), rvalue->toBoolean());
}

sp<Boolean> Disposed::__and__(const sp<Disposed>& lvalue, const sp<Boolean>& rvalue)
{
    return sp<BooleanAnd>::make(lvalue->toBoolean(), rvalue);
}

sp<Boolean> Disposed::__and__(const sp<Boolean>& lvalue, const sp<Disposed>& rvalue)
{
    return sp<BooleanAnd>::make(lvalue, rvalue->toBoolean());
}

sp<Boolean> Disposed::toBoolean() const
{
    return _disposed;
}

void Disposed::dispose()
{
    _disposed->set(true);
}

void Disposed::set(bool disposed)
{
    _disposed->set(disposed);
}

void Disposed::set(const sp<Boolean>& disposed)
{
    _disposed->set(disposed);
}

Disposed::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _disposed(value == "true")
{
    if(value && (value.at(0) == '@' || value.at(0) == '$'))
        _delegate = factory.ensureBuilder<Boolean>(value);
}

sp<Disposed> Disposed::DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate ? sp<Disposed>::make(_delegate->build(args)) : sp<Disposed>::make(_disposed);
}

template<> ARK_API sp<Disposed> Null::ptr()
{
    return Ark::instance().obtain<Disposed>(false);
}

}
