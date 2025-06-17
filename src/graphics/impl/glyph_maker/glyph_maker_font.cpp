#include "graphics/impl/glyph_maker/glyph_maker_font.h"

#include "core/inf/variable.h"

#include "graphics/base/glyph.h"

namespace ark {

GlyphMakerFont::GlyphMakerFont(sp<Font> font)
    : _font(std::move(font))
{
}

Vector<sp<Glyph>> GlyphMakerFont::makeGlyphs(const std::wstring& text)
{
    Vector<sp<Glyph>> glyphs;
    for(const wchar_t i : text)
        glyphs.push_back(sp<Glyph>::make(sp<Integer>::make<Integer::Const>(i), _font));
    return glyphs;
}

}
