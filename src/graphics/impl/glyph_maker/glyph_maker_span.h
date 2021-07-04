#ifndef ARK_GRAPHICS_IMPL_GLYPH_MAKER_GLYPH_MAKER_SPAN_H_
#define ARK_GRAPHICS_IMPL_GLYPH_MAKER_GLYPH_MAKER_SPAN_H_

#include "core/inf/builder.h"

#include "graphics/inf/glyph_maker.h"

namespace ark {

class GlyphMakerSpan : public GlyphMaker {
public:

    virtual std::vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override;

//[[plugin::builder]]
    class BUILDER : public Builder<GlyphMaker> {
    public:
        BUILDER() = default;

        virtual sp<GlyphMaker> build(const Scope& args) override;
    };

};

}

#endif
