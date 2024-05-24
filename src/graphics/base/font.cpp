#include "graphics/base/font.h"

#include "core/base/string.h"
#include "core/util/strings.h"

namespace ark {

Font::Font(const TextSize& size, Font::Family family, Font::Style style)
    : _size(size), _family(family), _style(style)
{
}

Font::Family Font::family() const
{
    return _family;
}

Font::Style Font::style() const
{
    return _style;
}

bool Font::operator <(const Font& other) const
{
    return _size < other._size || _family < other._family || _style < other._style;
}

const Font::TextSize& Font::size() const
{
    return _size;
}

Font::TextSize::TextSize()
    : _value(0), _unit(FONT_SIZE_UNIT_PX)
{
}

Font::TextSize::TextSize(const String& size)
{
    _unit = FONT_SIZE_UNIT_PT;

    if(size.endsWith("px"))
    {
        _unit = FONT_SIZE_UNIT_PX;
        _value = Strings::eval<uint32_t>(size.substr(0, size.length() - 2));
    }
    else if(size.endsWith("pt"))
        _value = Strings::eval<uint32_t>(size.substr(0, size.length() - 2));
    else
        _value = Strings::eval<uint32_t>(size);
}

Font::TextSize::TextSize(uint32_t value, Font::SizeUnit unit)
    : _value(value), _unit(unit)
{
}

bool Font::TextSize::operator <(const TextSize& other) const
{
    return _value < other._value || _unit < other._unit;
}

template<> ARK_API Font::TextSize StringConvert::eval<Font::TextSize>(const String& str)
{
    return Font::TextSize(str);
}

}
