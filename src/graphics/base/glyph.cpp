#include "graphics/base/glyph.h"

#include "graphics/base/font.h"
#include "graphics/components/render_object.h"
#include "graphics/components/size.h"
#include "graphics/util/transform_type.h"
#include "graphics/util/vec3_type.h"

namespace ark {

namespace {

class GlyphRenderObjectType final : public Integer {
public:
    GlyphRenderObjectType(sp<Integer> type, sp<Font> font)
        : _type(std::move(type)), _font(std::move(font))
    {
    }

    bool update(const uint64_t timestamp) override
    {
        return _type->update(timestamp);
    }

    int32_t val() override
    {
        return _font->combine(_type->val());
    }

private:
    sp<Integer> _type;
    sp<Font> _font;
};

}

Glyph::Glyph(sp<Integer> type, sp<Font> font, sp<Vec3> position, sp<Mat4> transform, sp<Varyings> varyings, sp<Boolean> visible, sp<Boolean> discarded)
    : _type(std::move(type)), _font(std::move(font)), _position(std::move(position)), _transform(std::move(transform)), _varyings(std::move(varyings)), _visible(std::move(visible)), _discarded(std::move(discarded))
{
}

const sp<Integer>& Glyph::type() const
{
    return _type;
}

const sp<Font>& Glyph::font() const
{
    return _font;
}

const sp<Vec3>& Glyph::position() const
{
    return _position;
}

const sp<Mat4>& Glyph::transform() const
{
    return _transform;
}

void Glyph::setTransform(sp<Mat4> transform)
{
    _transform = std::move(transform);
}

const sp<Varyings>& Glyph::varyings() const
{
    return _varyings;
}

void Glyph::setVaryings(sp<Varyings> varyings)
{
    _varyings = std::move(varyings);
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
    return sp<RenderObject>::make(_font ? sp<Integer>::make<GlyphRenderObjectType>(_type, _font) : _type, _position, sp<Size>::make(_content_size.x(), _content_size.y()), _transform ? TransformType::create(_transform) : sp<Transform>(), _varyings, _visible, _discarded);
}

wchar_t Glyph::character() const
{
    return static_cast<wchar_t>(_type->val());
}

const V2& Glyph::occupySize() const
{
    return _occupy_size;
}

const V2& Glyph::contentSize() const
{
    return _content_size;
}

void Glyph::setSize(const V2& occupySize, const V2& contentSize)
{
    _occupy_size = occupySize;
    _content_size = contentSize;
}

}
