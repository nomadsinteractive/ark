#include "graphics/base/layout_length.h"

#include "core/base/bean_factory.h"
#include "core/base/enum.h"
#include "core/util/strings.h"

namespace ark {

LayoutLength::LayoutLength()
    : _value(nullptr, 0), _type(LENGTH_TYPE_AUTO)
{
}

LayoutLength::LayoutLength(const float pixels)
    : LayoutLength(pixels, LENGTH_TYPE_PIXEL)
{
}

LayoutLength::LayoutLength(const float value, const LengthType type)
    : _value(sp<Numeric>::make<Numeric::Const>(value)), _type(type)
{
}

LayoutLength::LayoutLength(const String& value, const LengthType type)
    : _value(nullptr, 0), _type(type)
{
    if(value == "auto")
        return;

    if(value.endsWith("%"))
    {
        const String sNumber = value.substr(0, value.length() - 1);
        ASSERT(Strings::isNumeric(sNumber));
        ASSERT(_type == LENGTH_TYPE_AUTO || _type == LENGTH_TYPE_PERCENTAGE);
        _value.reset(Strings::eval<float>(sNumber));
        _type = LENGTH_TYPE_PERCENTAGE;
        return;
    }

    ASSERT(_type == LENGTH_TYPE_AUTO || _type == LENGTH_TYPE_PIXEL);
    _type = LENGTH_TYPE_PIXEL;
    if(value.endsWith("px"))
    {
        const String sNumber = value.substr(0, value.length() - 2);
        ASSERT(Strings::isNumeric(sNumber));
        _value.reset(Strings::eval<float>(sNumber));
    }
    else
    {
        ASSERT(Strings::isNumeric(value));
        _value.reset(Strings::eval<float>(value));
    }
}

LayoutLength::LayoutLength(sp<Numeric> value, const LengthType type)
    : _value(std::move(value)), _type(type)
{
}

bool LayoutLength::update(const uint64_t timestamp) const
{
    return _value.update(timestamp);
}

const SafeVar<Numeric>& LayoutLength::value() const
{
    return _value;
}

LayoutLength::LengthType LayoutLength::type() const
{
    return _type;
}

float LayoutLength::toPixelLength(const float parentLength) const
{
    return _type == LENGTH_TYPE_PIXEL ? _value.val() : _value.val() * parentLength / 100.0f;
}

bool LayoutLength::isMatchParent() const
{
    return _type == LENGTH_TYPE_PERCENTAGE && _value.val() == 100.0f;
}

LayoutLength::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _value(factory.ensureBuilder<Numeric>(manifest, constants::VALUE)), _type(Documents::getAttribute<LengthType>(manifest, constants::TYPE, LENGTH_TYPE_PIXEL))
{
}

LayoutLength LayoutLength::BUILDER::build(const Scope& args)
{
    return {_value->build(args), _type};
}

LayoutLength::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _value(value), _value_builder(Strings::isArgument(value) ? factory.ensureBuilder<Numeric>(value) : sp<Builder<Numeric>>())
{
}

LayoutLength LayoutLength::DICTIONARY::build(const Scope& args)
{
    if(_value_builder)
        return {_value_builder->build(args)};
    return {_value};
}

template<> ARK_API LayoutLength::LengthType StringConvert::eval<LayoutLength::LengthType>(const String& expr)
{
    constexpr Enum::LookupTable<LayoutLength::LengthType, 5> table = {{
        {"auto", LayoutLength::LENGTH_TYPE_AUTO},
        {"px", LayoutLength::LENGTH_TYPE_PIXEL},
        {"%", LayoutLength::LENGTH_TYPE_PERCENTAGE},
        {"pixel", LayoutLength::LENGTH_TYPE_PIXEL},
        {"percentage", LayoutLength::LENGTH_TYPE_PERCENTAGE}
    }};
    return Enum::lookup(table, expr);
}

}
