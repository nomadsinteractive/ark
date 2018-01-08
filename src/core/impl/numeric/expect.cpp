#include "core/impl/numeric/expect.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/message_loop.h"
#include "core/inf/runnable.h"
#include "core/util/math.h"
#include "core/util/bean_utils.h"

namespace ark {

Expect::Expect(const sp<Numeric>& delegate, const sp<Runnable>& onArrival, const sp<Numeric>& expectation)
    : _stub(sp<Stub>::make(delegate, onArrival, expectation))
{
}

float Expect::val()
{
    const sp<Stub> stub = _stub;
    if(stub->_delegate)
    {
        float value = stub->_delegate->val();
        float delta = stub->_expectation->val() - value;
        if(Math::signEquals(delta, stub->_delta) && std::abs(delta) > stub->_epsilon)
        {
            stub->_delta = delta;
            return value;
        }
        else
        {
            const sp<Runnable> onArrival = std::move(stub->_on_arrival);
            if(onArrival)
                Ark::instance().global<MessageLoop>()->post(onArrival, 0);
            stub->_delegate = nullptr;
        }
    }
    return stub->_expectation->val();
}

Expect::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _delegate(parent.ensureBuilder<Numeric>(doc, Constants::Attributes::DELEGATE)),
      _expectation(parent.ensureBuilder<Numeric>(doc, Constants::Attributes::VALUE)),
      _on_arrival(parent.getBuilder<Runnable>(doc, "onarrival"))
{
}

sp<Numeric> Expect::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Expect(_delegate->build(args), _on_arrival->build(args), _expectation->build(args)));
}

Expect::STYLE::STYLE(BeanFactory& beanFactory, const sp<Builder<Numeric>>& delegate, const String& value)
    : _delegate(delegate)
{
    BeanUtils::parse<Numeric, Runnable>(beanFactory, value, _expectation, _on_arrival);
    DCHECK(_expectation, "\"%s\"Error expectation format, should be (expectation[, on_arrival])", value.c_str());
}

sp<Numeric> Expect::STYLE::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Expect(_delegate->build(args), _on_arrival ? _on_arrival->build(args) : nullptr, _expectation->build(args)));
}

Expect::Stub::Stub(const sp<Numeric>& delegate, const sp<Runnable>& onArrival, const sp<Numeric>& expectataion)
    : _delegate(delegate), _on_arrival(onArrival), _expectation(expectataion), _delta(_expectation->val() - _delegate->val()), _epsilon(std::abs(_delta) / 100.0f)
{
}

}
