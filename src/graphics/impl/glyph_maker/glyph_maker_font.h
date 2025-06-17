#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/inf/glyph_maker.h"

namespace ark {

class GlyphMakerFont final : public GlyphMaker {
public:
    GlyphMakerFont(sp<Font> font);

    Vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override;

private:
    sp<Font> _font;
};

}
