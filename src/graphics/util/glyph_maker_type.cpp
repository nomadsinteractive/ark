#include "graphics/util/glyph_maker_type.h"

#include "graphics/base/glyph.h"
#include "graphics/impl/glyph_maker/glyph_maker_font.h"
#include "graphics/impl/glyph_maker/glyph_maker_span.h"

#include "renderer/components/varyings.h"

namespace ark {

namespace {

class GlyphMakerImpl final : public GlyphMaker {
public:
    GlyphMakerImpl(sp<Font> font, sp<Vec4> textColor)
        : _font(std::move(font))
    {
        if(textColor)
        {
            _varyings = sp<Varyings>::make();
            _varyings->setProperty("Color", std::move(textColor));
        }
    }

    Vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override
    {
        Vector<sp<Glyph>> glyphs;
        for(const wchar_t i : text)
            glyphs.push_back(sp<Glyph>::make(sp<Integer>::make<Integer::Const>(i), _font, nullptr, nullptr, _varyings));
        return glyphs;
    }

private:
    sp<Font> _font;
    sp<Varyings> _varyings;
};

class GlyphMakerTextColor final : public GlyphMaker {
public:
    GlyphMakerTextColor(sp<GlyphMaker> delegate, sp<Vec4> color)
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

}

sp<GlyphMaker> GlyphMakerType::create(sp<Font> font, sp<Vec4> color)
{
    ASSERT(font);
    if(color)
        return sp<GlyphMaker>::make<GlyphMakerImpl>(std::move(font), std::move(color));

    return sp<GlyphMaker>::make<GlyphMakerFont>(std::move(font));
}

sp<GlyphMaker> GlyphMakerType::setColor(sp<GlyphMaker> self, sp<Vec4> c)
{
    return sp<GlyphMaker>::make<GlyphMakerTextColor>(std::move(self), std::move(c));
}

sp<GlyphMaker> GlyphMakerType::setSpans(sp<GlyphMaker> self, Map<String, sp<GlyphMaker>> spans)
{
    return sp<GlyphMaker>::make<GlyphMakerSpan>(std::move(self), std::move(spans));
}

}
