#include "graphics/base/glyph.h"

#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/util/vec3_type.h"

namespace ark {

Glyph::Glyph(sp<Integer> type, sp<Vec3> position, sp<Transform> transform, sp<Varyings> varyings, sp<Boolean> visible, sp<Boolean> disposed)
    : _type(type), _position(std::move(position)), _transform(std::move(transform)), _varyings(std::move(varyings)), _visible(std::move(visible)), _disposed(std::move(disposed)), _character(0)
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

const sp<Boolean>& Glyph::disposed() const
{
    return _disposed;
}

const sp<Boolean>& Glyph::visible() const
{
    return _visible;
}

sp<RenderObject> Glyph::toRenderObject() const
{
    return sp<RenderObject>::make(_type, toRenderObjectPosition(), sp<Size>::make(_layout_size.x(), _layout_size.y()), _transform, _varyings, _visible, _disposed);
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

const V2& Glyph::layoutSize() const
{
    return _layout_size;
}

void Glyph::setLayoutSize(const V2& layoutSize)
{
    _layout_size = layoutSize;
}

}
