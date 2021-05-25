#ifndef ARK_GRAPHICS_IMPL_CHARACTER_MAKER_CHARACTER_MAKER_SPAN_H_
#define ARK_GRAPHICS_IMPL_CHARACTER_MAKER_CHARACTER_MAKER_SPAN_H_

#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"

#include "graphics/inf/character_maker.h"
#include "graphics/base/v2.h"

namespace ark {

class CharacterMakerSpan : public CharacterMaker {
public:
    CharacterMakerSpan(const V2& scale);
    CharacterMakerSpan(const V2& scale, BeanFactory& factory, const sp<String>& className);

    virtual std::vector<sp<RenderObject>> makeCharacter(const std::vector<Glyph>& glyphs) override;
    virtual V2 scale() override;

//[[plugin::builder]]
    class BUILDER : public Builder<CharacterMaker> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<CharacterMaker> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        SafePtr<Builder<String>> _class_name;
    };

private:
    V2 _scale;
    sp<Builder<RenderObject>> _character_builder;

};

}

#endif
