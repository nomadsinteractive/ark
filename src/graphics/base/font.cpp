#include "graphics/base/font.h"

#include "core/base/bean_factory.h"
#include "core/base/string.h"
#include "core/util/strings.h"
#include "core/base/constants.h"

namespace ark {

constexpr uint32_t FONT_SIZE_VALUE_BIT_SHIFT = 18;
constexpr uint32_t FONT_SIZE_VALUE_BITS = 10;
constexpr uint32_t FONT_SIZE_VALUE_MASK = (1 << (FONT_SIZE_VALUE_BITS + 1)) - 1;

constexpr uint32_t FONT_SIZE_UNIT_BIT_SHIFT = FONT_SIZE_VALUE_BIT_SHIFT + FONT_SIZE_VALUE_BITS;
constexpr uint32_t FONT_SIZE_UNIT_BITS = 1;

constexpr uint32_t FONT_STYLE_BIT_SHIFT = FONT_SIZE_UNIT_BIT_SHIFT + FONT_SIZE_UNIT_BITS;
constexpr uint32_t FONT_STYLE_BITS = 3;
constexpr uint32_t FONT_STYLE_MASK = (1 << (FONT_STYLE_BITS + 1)) - 1;

constexpr uint32_t UNICODE_MASK = (1 << (FONT_SIZE_VALUE_BIT_SHIFT + 1)) - 1;

Font::Font(const TextSize size, Style style)
    : _size(size), _style(style)
{
}

Font::Font(uint32_t size, SizeUnit sizeUnit, Style style)
    : _size(size, sizeUnit), _style(style)
{
}

Font::operator bool() const
{
    return _size._value != 0;
}

Font::TextSize Font::size() const
{
    return _size;
}

Font::Style Font::style() const
{
    return _style;
}

uint32_t Font::combine(uint32_t unicode) const
{
    ASSERT(unicode < 0x33000);
    return unicode | (_size._value & FONT_SIZE_VALUE_MASK) << FONT_SIZE_VALUE_BIT_SHIFT | (_size._unit & 1) << FONT_SIZE_UNIT_BIT_SHIFT | (_style & FONT_STYLE_MASK) << FONT_STYLE_BIT_SHIFT;
}

bool Font::operator<(const Font& other) const
{
    if(_size._value != other._size._value)
        return _size._value < other._size._value;

    if(_size._unit != other._size._unit)
        return _size._unit < other._size._unit;

    return _style < other._style;
}

std::pair<Font, uint32_t> Font::partition(uint32_t combined)
{
    const uint32_t unicode = combined & UNICODE_MASK;
    const uint32_t sizeValue = (combined >> FONT_SIZE_VALUE_BIT_SHIFT) & FONT_SIZE_VALUE_MASK;
    const SizeUnit sizeUnit = static_cast<SizeUnit>((combined >> FONT_SIZE_UNIT_BIT_SHIFT) & 1);
    const Style style = static_cast<Style>((combined >> FONT_STYLE_BIT_SHIFT) & FONT_STYLE_MASK);
    return {Font(sizeValue, sizeUnit, style), unicode};
}

Font::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _text_size(factory.ensureBuilder<String>(manifest, constants::SIZE))
{
}

sp<Font> Font::BUILDER::build(const Scope& args)
{
    return sp<Font>::make(Strings::eval<TextSize>(*_text_size->build(args)));
}

Font::DICTIONARY::DICTIONARY(BeanFactory& beanFactory, const String& expr)
    : _text_size(beanFactory.ensureBuilder<String>(expr))
{
}

sp<Font> Font::DICTIONARY::build(const Scope& args)
{
    return sp<Font>::make(Strings::eval<TextSize>(*_text_size->build(args)));
}

uint32_t Font::sizeValue() const
{
    return _size._value;
}

Font::SizeUnit Font::sizeUnit() const
{
    return _size._unit;
}

Font::TextSize::TextSize()
    : _value(0), _unit(FONT_SIZE_UNIT_PX)
{
}

Font::TextSize::TextSize(const String& size)
{
    _unit = FONT_SIZE_UNIT_PX;

    if(size.endsWith("pt"))
    {
        _unit = FONT_SIZE_UNIT_PT;
        _value = Strings::eval<uint32_t>(size.substr(0, size.length() - 2));
    }
    else if(size.endsWith("px"))
        _value = Strings::eval<uint32_t>(size.substr(0, size.length() - 2));
    else
        _value = Strings::eval<uint32_t>(size);
}

Font::TextSize::TextSize(uint32_t value, SizeUnit unit)
    : _value(value), _unit(unit)
{
}

template<> ARK_API Font::TextSize StringConvert::eval<Font::TextSize>(const String& str)
{
    return Font::TextSize(str);
}

}
