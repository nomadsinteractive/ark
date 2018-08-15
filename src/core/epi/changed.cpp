#include "core/epi/changed.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

namespace ark {

Changed::Changed(bool changed)
    : _stub(sp<Stub>::make(changed))
{
}

Changed::Changed(const sp<Boolean>& delegate)
    : _stub(sp<Stub>::make(delegate))
{
}

void Changed::change()
{
    _stub->_changed = true;
}

const sp<Boolean> Changed::toBoolean() const
{
    return _stub;
}

bool Changed::hasChanged()
{
    return _stub->val();
}

Changed::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _delegate(parent.ensureBuilder<Boolean>(doc))
{
}

sp<Changed> Changed::BUILDER::build(const sp<Scope>& args)
{
    return sp<Changed>::make(_delegate->build(args));
}

Changed::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _changed(value == "true")
{
    if(value && (value.at(0) == '@' || value.at(0) == '$'))
        _delegate = parent.ensureBuilder<Boolean>(value);
}

sp<Changed> Changed::DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate ? sp<Changed>::make(_delegate->build(args)) : sp<Changed>::make(_changed);
}

Changed::Stub::Stub(bool changed)
    : _changed(changed)
{
}

Changed::Stub::Stub(const sp<Boolean>& delegate)
    : _changed(false), _delegate(delegate)
{
}

bool Changed::Stub::val()
{
    bool c = _changed || (_delegate && _delegate->val());
    if(c)
        _changed = false;
    return c;
}

}
