#include "graphics/base/glyph.h"

#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
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
    return sp<RenderObject>::make(_type, toRenderObjectPosition(), sp<Size>::make(_occupy_size.x(), _occupy_size.y()), _transform, _varyings, _visible, _discarded);
}

sp<Vec3> Glyph::toRenderObjectPosition() const
{
    sp<Vec3> layoutPosition = sp<Vec3>::make<Vec3::Const>(_layout_position);
    return _position ? Vec3Type::add(_position, std::move(layoutPosition)) : layoutPosition;
}

wchar_t Glyph::character() const
{
    return _character;
}

void Glyph::setCharacter(wchar_t character)
{
    _character = character;
}

const V3& Glyph::layoutPosition() const
{
    return _layout_position;
}

void Glyph::setLayoutPosition(const V3& layoutPosition)
{
    _layout_position = layoutPosition;
}

const V2& Glyph::occupySize() const
{
    return _occupy_size;
}

void Glyph::setOccupySize(const V2& layoutSize)
{
    _occupy_size = layoutSize;
}

}
