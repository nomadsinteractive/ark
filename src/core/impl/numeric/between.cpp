#include "core/impl/numeric/between.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/observer.h"
#include "core/util/updatable_util.h"

namespace ark {

Between::Between(const sp<Numeric>& delegate, const sp<Numeric>& min, const sp<Numeric>& max, const sp<Observer>& observer)
    : _delegate(delegate), _min(min), _max(max), _observer(observer), _value(delegate->val()), _at_border(_value == min->val() || _value == max->val())
{
}

float Between::val()
{
    return _value;
}

bool Between::update(uint64_t timestamp)
{
    if(!UpdatableUtil::update(timestamp, _delegate, _min, _max))
        return false;

    _value = _delegate->val();
    float min = _min->val();
    float max = _max->val();
    bool crossed = false;
    if(_at_border)
    {
        crossed = _value < min || _value > max;
        if(!crossed)
            _at_border = _value == min || _value == max;
    }
    else
        crossed = _value <= min || _value >= max;
    if(crossed)
        _observer->notify();

    return true;
}

Between::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory.ensureBuilder<Numeric>(manifest, constants::DELEGATE)),
      _min(factory.ensureBuilder<Numeric>(manifest, "min")),
      _max(factory.ensureBuilder<Numeric>(manifest, "max")),
      _observer(factory.ensureBuilder<Observer>(manifest, "observer"))
{
}

sp<Numeric> Between::BUILDER::build(const Scope& args)
{
    return sp<Numeric>::adopt(new Between(_delegate->build(args), _min->build(args), _max->build(args), _observer->build(args)));
}

}
