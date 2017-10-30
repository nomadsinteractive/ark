#include "graphics/base/font.h"

namespace ark {

Font::Font(Font::Family family, Font::Style style, uint32_t size)
    : _family(family), _style(style), _size(size)
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

uint32_t Font::size() const
{
    return _size;
}

}
