#ifndef ARK_GRAPHICS_IMPL_CHARACTER_MAKER_CHARACTER_MAKER_SPAN_H_
#define ARK_GRAPHICS_IMPL_CHARACTER_MAKER_CHARACTER_MAKER_SPAN_H_

#include "core/base/bean_factory.h"
#include "core/inf/builder.h"

#include "graphics/inf/character_maker.h"
#include "graphics/base/v2.h"

namespace ark {

class CharacterMakerSpan : public CharacterMaker {
public:
    CharacterMakerSpan(const V2& scale);
    CharacterMakerSpan(const V2& scale, BeanFactory& factory, const sp<String>& className);

    virtual sp<RenderObject> makeCharacter(int32_t type, const V3& position, const sp<Size>& size) override;
    virtual V2 scale() override;

//[[plugin::builder]]
    class BUILDER : public Builder<CharacterMaker> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<CharacterMaker> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<String>> _class_name;

    };

private:
    V2 _scale;
    sp<Builder<RenderObject>> _character_builder;

};

}

#endif