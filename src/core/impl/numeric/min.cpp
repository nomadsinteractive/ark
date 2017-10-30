#include "core/impl/numeric/min.h"

#include "core/base/bean_factory.h"

namespace ark {

Min::Min(const sp<Numeric>& delegate, const sp<Numeric>& min)
    : _delegate(delegate), _min(min)
{
}

float Min::val()
{
    float v = _delegate->val();
    float min = _min->val();
    return v >= min ? v : min;
}

sp<Numeric> Min::min(const sp<Numeric>& a, const sp<Numeric>& b)
{
    return sp<Numeric>::adopt(new Min(a, b));
}

Min::DECORATOR::DECORATOR(BeanFactory& parent, const sp<Builder<Numeric>>& delegate, const String& value)
    : _delegate(delegate), _min(parent.ensureBuilder<Numeric>(value))
{
}

sp<Numeric> Min::DECORATOR::build(const sp<Scope>& args)
{
    return sp<Min>::make(_delegate->build(args), _min->build(args));
}

}
