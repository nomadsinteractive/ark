#ifndef ARK_GRAPHICS_IMPL_CHARACTER_MAKER_CHARACTER_MAKER_TEXT_COLOR_H_
#define ARK_GRAPHICS_IMPL_CHARACTER_MAKER_CHARACTER_MAKER_TEXT_COLOR_H_

#include "core/inf/builder.h"

#include "graphics/inf/character_maker.h"

namespace ark {

class CharacterMakerTextColor : public CharacterMaker {
public:
    CharacterMakerTextColor(sp<CharacterMaker> delegate, sp<Vec4> color);

    virtual std::vector<sp<RenderObject>> makeCharacter(const std::vector<Glyph>& glyphs) override;
    virtual V2 scale() override;

//  [[plugin::style("text-color")]]
    class BUILDER : public Builder<CharacterMaker> {
    public:
        BUILDER(BeanFactory& beanFactory, const sp<Builder<CharacterMaker>>& delegate, const String& style);

        virtual sp<CharacterMaker> build(const Scope& args) override;

    private:
        sp<Builder<CharacterMaker>> _delegate;
        sp<Builder<Vec4>> _color;

    };

private:
    sp<CharacterMaker> _delegate;
    sp<Vec4> _color;
};

}

#endif
