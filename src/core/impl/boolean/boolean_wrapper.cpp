#include "core/impl/boolean/boolean_wrapper.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"
#include "core/util/identifier.h"

namespace ark {

BooleanWrapper::BooleanWrapper(bool value)
    : _value(value)
{
}

BooleanWrapper::BooleanWrapper(const sp<Boolean>& delegate)
    : _delegate(delegate), _value(false)
{
}

BooleanWrapper::BooleanWrapper(const BooleanWrapper& other)
    : _delegate(other._delegate), _value(other._value)
{
}

BooleanWrapper::BooleanWrapper(BooleanWrapper&& other)
    : _delegate(std::move(other._delegate)), _value(other._value)
{
}

bool BooleanWrapper::val()
{
    return _delegate ? _delegate->val() : _value;
}

void BooleanWrapper::set(bool value)
{
    _delegate = nullptr;
    _value = value;
}

void BooleanWrapper::setDelegate(const sp<Boolean>& delegate)
{
    _delegate = delegate;
}

BooleanWrapper::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _delegate(parent.ensureBuilder<Boolean>(doc, Constants::Attributes::VALUE))
{
}

sp<Boolean> BooleanWrapper::BUILDER::build(const sp<Scope>& args)
{
    return sp<BooleanWrapper>::make(_delegate->build(args));
}

}
