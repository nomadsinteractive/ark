#include "graphics/base/glyph.h"

namespace ark {

Glyph::Glyph(int32_t character, const V3& position, const V2& size)
    : _character(character), _position(position), _size(size)
{
}

int32_t Glyph::character() const
{
    return _character;
}

const V3& Glyph::position() const
{
    return _position;
}

const V2& Glyph::size() const
{
    return _size;
}

}
