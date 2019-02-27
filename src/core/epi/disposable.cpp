#include "core/epi/disposable.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"

namespace ark {

Disposable::Disposable(bool disposed)
    : _disposed(sp<BooleanWrapper>::make(disposed))
{
}

Disposable::Disposable(const sp<Boolean>& disposed)
    : _disposed(sp<BooleanWrapper>::make(disposed))
{
}

bool Disposable::isDisposed() const
{
    return _disposed->val();
}

sp<Boolean> Disposable::toBoolean() const
{
    return _disposed;
}

void Disposable::dispose()
{
    _disposed->set(true);
}

Disposable::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _disposed(value == "true")
{
    if(value && (value.at(0) == '@' || value.at(0) == '$'))
        _delegate = parent.ensureBuilder<Boolean>(value);
}

sp<Disposable> Disposable::DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate ? sp<Disposable>::make(_delegate->build(args)) : sp<Disposable>::make(_disposed);
}

}
