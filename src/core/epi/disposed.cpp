#include "core/epi/disposed.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_observer.h"
#include "core/impl/variable/variable_wrapper.h"

namespace ark {

Disposed::Disposed(bool disposed)
    : _disposed(sp<BooleanWrapper>::make(disposed))
{
}

Disposed::Disposed(sp<Boolean> disposed)
    : _disposed(sp<BooleanWrapper>::make(disposed))
{
}

bool Disposed::val()
{
    return _disposed->val();
}

bool Disposed::update(uint64_t timestamp)
{
    return _disposed->update(timestamp);
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

const sp<Boolean>& Disposed::delegate() const
{
    return _disposed->wrapped();
}

Disposed::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _disposed(value == "true")
{
    if(value && (value.at(0) == '@' || value.at(0) == '$'))
        _delegate = factory.ensureBuilder<Boolean>(value);
}

sp<Disposed> Disposed::DICTIONARY::build(const Scope& args)
{
    return _delegate ? sp<Disposed>::make(_delegate->build(args)) : sp<Disposed>::make(_disposed);
}

template<> ARK_API sp<Disposed> Null::ptr()
{
    return sp<Disposed>::make(false);
}

}
