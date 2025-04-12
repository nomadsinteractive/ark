#include "graphics/impl/glyph_maker/glyph_maker_span.h"

#include "core/inf/variable.h"

#include "graphics/base/glyph.h"

namespace ark {

GlyphMakerSpan::GlyphMakerSpan(sp<Font> font)
    : _font(std::move(font))
{
}

Vector<sp<Glyph>> GlyphMakerSpan::makeGlyphs(const std::wstring& text)
{
    Vector<sp<Glyph>> glyphs;
    for(const wchar_t i : text)
        glyphs.push_back(sp<Glyph>::make(sp<Integer>::make<Integer::Const>(i), _font));
    return glyphs;
}

}
