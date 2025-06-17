#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("GlyphMaker")]]
class ARK_API GlyphMakerType {
public:
//  [[script::bindings::constructor]]
    static sp<GlyphMaker> create(sp<Font> font, sp<Vec4> color = nullptr);

//  [[script::bindings::classmethod]]
    static sp<GlyphMaker> setColor(sp<GlyphMaker> self, sp<Vec4> c);
//  [[script::bindings::classmethod]]
    static sp<GlyphMaker> setSpans(sp<GlyphMaker> self, Map<String, sp<GlyphMaker>> spans);

};

}
