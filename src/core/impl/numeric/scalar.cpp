#include "core/impl/numeric/scalar.h"

#include <cmath>

#include "core/ark.h"

#include "core/types/null.h"
#include "core/util/bean_utils.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"

namespace ark {

Scalar::Scalar(float value)
    : _value(value)
{
}

Scalar::Scalar(const sp<Numeric>& delegate)
    : _delegate(Null::toSafe(delegate)), _value(NAN)
{
}

Scalar::Scalar(const Scalar& other)
    : _delegate(other._delegate), _value(other._value)
{
}

Scalar::Scalar(Scalar&& other)
    : _delegate(std::move(other._delegate)), _value(other._value)
{
}

float Scalar::val()
{
    return std::isnan(_value) ? _delegate->val() : _value;
}

void Scalar::set(float value)
{
    _value = value;
    deferedUnref();
    _delegate = nullptr;
}

void Scalar::fix()
{
    if(_delegate)
        _value = _delegate->val();
    deferedUnref();
    _delegate = nullptr;
}

const sp<Numeric>& Scalar::delegate()
{
    return _delegate;
}

void Scalar::setDelegate(const sp<Numeric>& delegate)
{
    deferedUnref();
    _delegate = delegate;
    _value = NAN;
}

void Scalar::deferedUnref()
{
    Ark::instance().applicationContext()->deferUnref(_delegate);
}

void Scalar::assign(const sp<Scalar>& other)
{
    deferedUnref();
    if(other)
    {
        _delegate = other->_delegate;
        _value = other->_value;
    }
    else
    {
        _delegate = Null::ptr<Numeric>();
        _value = NAN;
    }
}

Scalar::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _value(NAN)
{
    const String& idstr = Documents::ensureAttribute(doc, Constants::Attributes::VALUE);
    const Identifier id = Identifier::parse(idstr);
    if(id.isVal() && Strings::isNumeric(id.val()))
        _value = Strings::parse<float>(id.val());
    else
        _delegate = parent.ensureBuilder<Numeric>(idstr);
}

sp<Numeric> Scalar::BUILDER::build(const sp<Scope>& args)
{
    return _delegate ? sp<Scalar>::make(_delegate->build(args)) : sp<Scalar>::make(_value);
}

}
