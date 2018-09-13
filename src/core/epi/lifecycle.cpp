#include "core/epi/lifecycle.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"

namespace ark {

Lifecycle::Lifecycle(bool disposed)
    : _disposed(sp<BooleanWrapper>::make(disposed))
{
}

Lifecycle::Lifecycle(const sp<Boolean>& disposed)
    : _disposed(sp<BooleanWrapper>::make(disposed))
{
}

bool Lifecycle::val()
{
    return _disposed->val();
}

void Lifecycle::dispose()
{
    _disposed->set(true);
}

Lifecycle::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _disposed(value == "true")
{
    if(value && (value.at(0) == '@' || value.at(0) == '$'))
        _delegate = parent.ensureBuilder<Boolean>(value);
}

sp<Lifecycle> Lifecycle::DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate ? sp<Lifecycle>::make(_delegate->build(args)) : sp<Lifecycle>::make(_disposed);
}

}
