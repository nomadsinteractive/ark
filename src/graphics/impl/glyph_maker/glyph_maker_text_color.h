#ifndef ARK_GRAPHICS_IMPL_GLYPH_MAKER_GLYPH_MAKER_TEXT_COLOR_H_
#define ARK_GRAPHICS_IMPL_GLYPH_MAKER_GLYPH_MAKER_TEXT_COLOR_H_

#include "core/inf/builder.h"

#include "graphics/inf/glyph_maker.h"

namespace ark {

class CharacterMakerTextColor : public GlyphMaker {
public:
    CharacterMakerTextColor(sp<GlyphMaker> delegate, sp<Vec4> color);

    virtual std::vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override;

//  [[plugin::style("text-color")]]
    class BUILDER : public Builder<GlyphMaker> {
    public:
        BUILDER(BeanFactory& beanFactory, const sp<Builder<GlyphMaker>>& delegate, const String& style);

        virtual sp<GlyphMaker> build(const Scope& args) override;

    private:
        sp<Builder<GlyphMaker>> _delegate;
        sp<Builder<Vec4>> _color;

    };

private:
    sp<GlyphMaker> _delegate;
    sp<Varyings> _varyings;
    sp<Vec4> _color;
};

}

#endif
