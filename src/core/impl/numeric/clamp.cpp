#include "core/impl/numeric/clamp.h"

#include "core/base/bean_factory.h"
#include "core/inf/runnable.h"
#include "core/util/bean_utils.h"

namespace ark {

Clamp::Clamp(const sp<Numeric>& delegate, const sp<Numeric>& min, const sp<Numeric>& max, const sp<Runnable>& notifier)
    : _delegate(delegate), _min(min), _max(max), _notifier(notifier)
{
}

float Clamp::val()
{
    float v = _delegate->val();
    float t;
    if((t = _min->val()) > v)
    {
        _notifier.notify();
        return t;
    }
    if((t = _max->val()) < v)
    {
        _notifier.notify();
        return t;
    }
    return v;
}

Clamp::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory.ensureBuilder<Numeric>(manifest, Constants::Attributes::DELEGATE)), _min(factory.ensureBuilder<Numeric>(manifest, "min")),
      _max(factory.ensureBuilder<Numeric>(manifest, "max"))
{
}

sp<Numeric> Clamp::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Clamp(_delegate->build(args), _min->build(args), _max->build(args), nullptr));
}

Clamp::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Numeric>>& delegate, const String& style)
    : _delegate(delegate)
{
    BeanUtils::parse<Numeric, Numeric, Runnable>(factory, style, _min, _max, _notifier);
}

sp<Numeric> Clamp::STYLE::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Clamp(_delegate->build(args), _min->build(args), _max->build(args), _notifier ? _notifier->build(args) : nullptr));
}

}
