#include "core/impl/numeric/between.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/message_loop.h"

namespace ark {

Between::Between(const sp<Numeric>& delegate, const sp<Numeric>& min, const sp<Numeric>& max, const sp<Runnable>& oncross)
    : _delegate(delegate), _min(min), _max(max), _oncross(oncross), _at_border(delegate->val() == min->val() || delegate->val() == max->val())
{
}

float Between::val()
{
    float value = _delegate->val();
    if(_oncross)
    {
        float min = _min->val();
        float max = _max->val();
        bool crossed = false;
        if(_at_border)
        {
            crossed = value < min || value > max;
            if(!crossed)
                _at_border = value == min || value == max;
        }
        else
            crossed = value <= min || value >= max;
        if(crossed)
        {
            const sp<Runnable> onCross = std::move(_oncross);
            Ark::instance().ensure<MessageLoop>()->post(onCross, 0);
        }
    }
    return value;
}

Between::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory.ensureBuilder<Numeric>(manifest, Constants::Attributes::DELEGATE)),
      _min(factory.ensureBuilder<Numeric>(manifest, "min")),
      _max(factory.ensureBuilder<Numeric>(manifest, "max")),
      _oncross(factory.ensureBuilder<Runnable>(manifest, Constants::Attributes::ON_CROSS))
{
}

sp<Numeric> Between::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Between(_delegate->build(args), _min->build(args), _max->build(args), _oncross->build(args)));
}

}
