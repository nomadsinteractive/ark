#include "core/traits/disposed.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_observer.h"
#include "core/impl/variable/variable_wrapper.h"

namespace ark {

Disposed::Disposed(bool disposed)
    : _discarded(sp<BooleanWrapper>::make(disposed))
{
}

Disposed::Disposed(sp<Boolean> disposed)
    : _discarded(sp<BooleanWrapper>::make(disposed))
{
}

bool Disposed::val()
{
    return _discarded->val();
}

bool Disposed::update(uint64_t timestamp)
{
    return _discarded->update(timestamp);
}

void Disposed::dispose()
{
    _discarded->set(true);
}

void Disposed::set(bool disposed)
{
    _discarded->set(disposed);
}

void Disposed::set(sp<Boolean> disposed)
{
    _discarded->set(std::move(disposed));
}

const sp<Boolean>& Disposed::wrapped() const
{
    return _discarded->wrapped();
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

template<> ARK_API sp<Disposed> Null::safePtr()
{
    return sp<Disposed>::make(false);
}

}
