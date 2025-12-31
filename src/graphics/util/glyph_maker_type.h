#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("GlyphMaker")]]
class ARK_API GlyphMakerType {
public:
//  [[script::bindings::constructor]]
    static sp<GlyphMaker> create(sp<Font> font = nullptr);

//  [[script::bindings::classmethod]]
    static sp<GlyphMaker> withColor(sp<GlyphMaker> self, sp<Vec4> c);
//  [[script::bindings::classmethod]]
    static sp<GlyphMaker> withSpans(sp<GlyphMaker> self, Map<String, sp<GlyphMaker>> spans);
//  [[script::bindings::classmethod]]
    static sp<GlyphMaker> withTransform(sp<GlyphMaker> self, sp<Mat4> transform);
//  [[script::bindings::classmethod]]
    static sp<GlyphMaker> withVaryings(sp<GlyphMaker> self, sp<Varyings> varyings);
//  [[script::bindings::classmethod]]
    static sp<GlyphMaker> withFont(sp<GlyphMaker> self, sp<Font> font);
};

}
