#include "core/impl/numeric/envelope.h"

#include "core/dom/document.h"
#include "core/base/expectation.h"
#include "core/impl/boolean/boolean_or.h"
#include "core/util/math.h"

namespace ark {

Envelope::Envelope(BeanFactory& factory, const document& manifest, const sp<Scope>& args)
    : _stub(sp<Stub>::make(factory, manifest, args))
{
    _phrases_iterator = _stub->_phrases.begin();
    _expired = buildState();
}

float Envelope::val()
{
    while(_expired->val())
    {
        if(_phrases_iterator != _stub->_phrases.end())
            ++_phrases_iterator;

        _expired = Null::ptr<Boolean>();
        if(_phrases_iterator != _stub->_phrases.end())
            _expired = buildState();
    }

    return _stub->_value->val();
}

Envelope::Phrase::Phrase(BeanFactory& factory, const document& manifest)
    : _value(factory.ensureBuilder<Numeric>(manifest)), _expired(factory.getBuilder<Boolean>(manifest, Constants::Attributes::EXPIRED, false)),
      _expectation(factory.getBuilder<Expectation>(manifest, Constants::Attributes::EXPECTATION, false))
{
}

Envelope::Stub::Stub(BeanFactory& factory, const document& manifest, const sp<Scope>& args)
    : _args(args)
{
    for(const document& i : manifest->children())
        _phrases.push_back(Phrase(factory, i));
}

sp<Boolean> Envelope::buildState()
{
    const Phrase& phrase = *_phrases_iterator;
    sp<Boolean> expired;
    _stub->_value = phrase._value->build(_stub->_args);
    if(phrase._expired)
        expired = phrase._expired->build(_stub->_args);
    if(phrase._expectation)
    {
        const sp<Expectation> expectation = phrase._expectation->build(_stub->_args);
        const sp<Boolean> e = sp<ExpectedExpirable>::make(_stub, expectation);
        expired = expired ? sp<Boolean>::adopt(new BooleanOr(expired, e)) : e;
    }
    return Null::toSafe(expired);
}

Envelope::ExpectedExpirable::ExpectedExpirable(const sp<Envelope::Stub>& stub, const sp<Expectation>& expectation)
    : _stub(stub), _expectation(expectation)
{
}

bool Envelope::ExpectedExpirable::val()
{
    float v1 = _stub->_value->val();
    float v2 = _expectation->val();
    bool equal = Math::almostEqual(v1, v2, 2);
    if(equal)
        _expectation->fire();
    return equal;
}

Envelope::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest)
{
}

sp<Numeric> Envelope::BUILDER::build(const sp<Scope>& args)
{
    return sp<Envelope>::make(_factory, _manifest, args);
}

}
