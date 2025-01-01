#include "graphics/base/glyph.h"

#include "graphics/base/render_object.h"
#include "graphics/traits/size.h"
#include "graphics/util/vec3_type.h"

namespace ark {

Glyph::Glyph(sp<Integer> type, sp<Vec3> position, sp<Transform> transform, sp<Varyings> varyings, sp<Boolean> visible, sp<Boolean> discarded)
    : _type(type), _position(std::move(position)), _transform(std::move(transform)), _varyings(std::move(varyings)), _visible(std::move(visible)), _discarded(std::move(discarded)), _character(0)
{
}

const sp<Integer>& Glyph::type() const
{
    return _type;
}

const sp<Vec3>& Glyph::position() const
{
    return _position;
}

const sp<Transform>& Glyph::transform() const
{
    return _transform;
}

const sp<Varyings>& Glyph::varyings() const
{
    return _varyings;
}

void Glyph::setVaryings(const sp<Varyings>& varyings)
{
    _varyings = varyings;
}

const sp<Boolean>& Glyph::discarded() const
{
    return _discarded;
}

const sp<Boolean>& Glyph::visible() const
{
    return _visible;
}

sp<RenderObject> Glyph::toRenderObject() const
{
    return sp<RenderObject>::make(_type, _position, sp<Size>::make(_content_size.x(), _content_size.y()), _transform, _varyings, _visible, _discarded);
}

wchar_t Glyph::character() const
{
    return _character;
}

void Glyph::setCharacter(wchar_t character)
{
    _character = character;
}

const V2& Glyph::occupySize() const
{
    return _occupy_size;
}

void Glyph::setSize(const V2& occupySize, const V2& contentSize)
{
    _occupy_size = occupySize;
    _content_size = contentSize;
}

}
