#pragma once

#include "core/inf/builder.h"

#include "graphics/inf/glyph_maker.h"

namespace ark {

class GlyphMakerSpan final : public GlyphMaker {
public:
    GlyphMakerSpan(sp<Font> font);

    std::vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override;

private:
    sp<Font> _font;
};

}
