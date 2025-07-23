#include "graphics/util/glyph_maker_type.h"

#include "graphics/base/glyph.h"
#include "graphics/impl/glyph_maker/glyph_maker_font.h"
#include "graphics/impl/glyph_maker/glyph_maker_span.h"
#include "graphics/util/mat4_type.h"

#include "renderer/components/varyings.h"

namespace ark {

namespace {

class GlyphMakerWithColor final : public GlyphMaker {
public:
    GlyphMakerWithColor(sp<GlyphMaker> delegate, sp<Vec4> color)
        : _delegate(delegate ? std::move(delegate) : sp<GlyphMaker>::make<GlyphMakerFont>(nullptr)), _varyings(sp<Varyings>::make()), _color(std::move(color))
    {
        _varyings->setProperty("Color", _color);
    }

    Vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override
    {
        Vector<sp<Glyph>> glyphs = _delegate->makeGlyphs(text);
        for(const sp<Glyph>& i : glyphs)
            if(i->varyings())
                i->varyings()->setProperty("Color", _color);
            else
                i->setVaryings(_varyings);
        return glyphs;
    }

private:
    sp<GlyphMaker> _delegate;
    sp<Varyings> _varyings;
    sp<Vec4> _color;
};

class GlyphMakerWithTransform final : public GlyphMaker {
public:
    GlyphMakerWithTransform(sp<GlyphMaker> delegate, sp<Mat4> transform)
        : _delegate(std::move(delegate)), _transform(std::move(transform))
    {
    }

    Vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override
    {
        Vector<sp<Glyph>> glyphs = _delegate->makeGlyphs(text);
        for(const sp<Glyph>& i : glyphs)
            i->setTransform(i->transform() ? Mat4Type::matmul(_transform, i->transform()) : _transform);
        return glyphs;
    }

private:
    sp<GlyphMaker> _delegate;
    sp<Mat4> _transform;
};

}

sp<GlyphMaker> GlyphMakerType::create(sp<Font> font)
{
    ASSERT(font);
    return sp<GlyphMaker>::make<GlyphMakerFont>(std::move(font));
}

sp<GlyphMaker> GlyphMakerType::withColor(sp<GlyphMaker> self, sp<Vec4> c)
{
    return sp<GlyphMaker>::make<GlyphMakerWithColor>(std::move(self), std::move(c));
}

sp<GlyphMaker> GlyphMakerType::withSpans(sp<GlyphMaker> self, Map<String, sp<GlyphMaker>> spans)
{
    return sp<GlyphMaker>::make<GlyphMakerSpan>(std::move(self), std::move(spans));
}

sp<GlyphMaker> GlyphMakerType::withTransform(sp<GlyphMaker> self, sp<Mat4> transform)
{
    return sp<GlyphMaker>::make<GlyphMakerWithTransform>(std::move(self), std::move(transform));
}

}
