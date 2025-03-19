#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/impl/builder/safe_builder.h"

#include "graphics/inf/glyph_maker.h"

namespace ark {

//[[script::bindings::class("GlyphMaker")]]
class ARK_API GlyphMakerType {
public:


//  [[plugin::builder]]
    class BUILDER final : public Builder<GlyphMaker> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<GlyphMaker> build(const Scope& args) override;

    private:
        SafeBuilder<Font> _font;
        SafeBuilder<Vec4> _text_color;
    };
};

}
