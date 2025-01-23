#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/inf/glyph_maker.h"

namespace ark {

class CharacterMakerTextColor final : public GlyphMaker {
public:
    CharacterMakerTextColor(sp<GlyphMaker> delegate, sp<Vec4> color);

    Vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override;

private:
    sp<GlyphMaker> _delegate;
    sp<Varyings> _varyings;
    sp<Vec4> _color;
};

}
