#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/inf/glyph_maker.h"

namespace ark {

class GlyphMakerSpan final : public GlyphMaker {
public:
    GlyphMakerSpan(sp<GlyphMaker> glyphMakerDefault, Map<String, sp<GlyphMaker>> spans);

    Vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override;

private:
    sp<GlyphMaker> _glyph_maker_default;
    Map<String, sp<GlyphMaker>> _glyph_maker_spans;
};

}
