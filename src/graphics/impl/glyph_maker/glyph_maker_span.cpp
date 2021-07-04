#include "graphics/impl/glyph_maker/glyph_maker_span.h"

#include "core/inf/variable.h"

#include "graphics/base/glyph.h"

namespace ark {

std::vector<sp<Glyph>> GlyphMakerSpan::makeGlyphs(const std::wstring& text)
{
    std::vector<sp<Glyph>> glyphs;
    for(const wchar_t i : text)
        glyphs.push_back(sp<Glyph>::make(sp<Integer>::make<Integer::Const>(i)));
    return glyphs;
}

sp<GlyphMaker> GlyphMakerSpan::BUILDER::build(const Scope& /*args*/)
{
    return sp<GlyphMakerSpan>::make();
}

}
