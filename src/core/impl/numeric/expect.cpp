#include "core/impl/numeric/expect.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/expectation.h"
#include "core/inf/message_loop.h"
#include "core/inf/runnable.h"
#include "core/util/math.h"
#include "core/util/bean_utils.h"

namespace ark {

Expect::Expect(const sp<Numeric>& delegate, const sp<Expectation>& expectation)
    : _stub(sp<Stub>::make(delegate, expectation))
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
            stub->_expectation->fireOnce();
    }
    return stub->_expectation->val();
}

Expect::BUILDER::BUILDER(BeanFactory& factory, const document& doc)
    : _delegate(factory.ensureBuilder<Numeric>(doc, Constants::Attributes::DELEGATE)),
      _expectation(factory.ensureBuilder<Expectation>(doc, Constants::Attributes::EXPECT))
{
}

sp<Numeric> Expect::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Expect(_delegate->build(args), _expectation->build(args)));
}

Expect::STYLE::STYLE(BeanFactory& beanFactory, const sp<Builder<Numeric>>& delegate, const String& value)
    : _delegate(delegate), _expectation(beanFactory.ensureBuilder<Expectation>(value))
{
}

sp<Numeric> Expect::STYLE::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Expect(_delegate->build(args), _expectation->build(args)));
}

Expect::Stub::Stub(const sp<Numeric>& delegate, const sp<Expectation>& expectation)
    : _delegate(delegate), _expectation(expectation), _delta(_expectation->val() - _delegate->val()), _epsilon(std::abs(_delta) / 100.0f)
{
}

}
