#include "core/epi/changed.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

namespace ark {

Changed::Changed(bool changed)
    : _changed(changed)
{
}

Changed::Changed(const sp<Boolean>& delegate)
    : _changed(false), _delegate(delegate)
{
}

void Changed::change()
{
    _changed = true;
}

bool Changed::hasChanged()
{
    bool c = _changed || (_delegate && _delegate->val());
    if(c)
        _changed = false;
    return c;
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

}
