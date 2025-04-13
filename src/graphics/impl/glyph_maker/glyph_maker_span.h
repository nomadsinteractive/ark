#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/inf/glyph_maker.h"

namespace ark {

class GlyphMakerSpan final : public GlyphMaker {
public:
    GlyphMakerSpan(sp<Font> font);

    Vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override;

private:
    sp<Font> _font;
};

}
