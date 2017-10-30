#include "core/impl/numeric/max.h"

#include "core/base/bean_factory.h"

namespace ark {

Max::Max(const sp<Numeric>& delegate, const sp<Numeric>& max)
    : _delegate(delegate), _max(max)
{
}

float Max::val()
{
    float v = _delegate->val();
    float max = _max->val();
    return v <= max ? v : max;
}

sp<Numeric> Max::max(const sp<Numeric>& a, const sp<Numeric>& b)
{
    return sp<Numeric>::adopt(new Max(a, b));
}

Max::DECORATOR::DECORATOR(BeanFactory& parent, const sp<Builder<Numeric>>& delegate, const String& value)
    : _delegate(delegate), _max(parent.ensureBuilder<Numeric>(value))
{
}

sp<Numeric> Max::DECORATOR::build(const sp<Scope>& args)
{
    return sp<Max>::make(_delegate->build(args), _max->build(args));
}

}
