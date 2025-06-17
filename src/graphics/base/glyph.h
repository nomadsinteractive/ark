#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Glyph {
public:
//  [[script::bindings::auto]]
    Glyph(sp<Integer> type, sp<Font> font = nullptr, sp<Vec3> position = nullptr, sp<Transform> transform = nullptr, sp<Varyings> varyings = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr);

//  [[script::bindings::property]]
    const sp<Integer>& type() const;
//  [[script::bindings::property]]
    const sp<Font>& font() const;
//  [[script::bindings::property]]
    const sp<Vec3>& position() const;
//  [[script::bindings::property]]
    const sp<Transform>& transform() const;
//  [[script::bindings::property]]
    const sp<Varyings>& varyings() const;
//  [[script::bindings::property]]
    void setVaryings(sp<Varyings> varyings);
//  [[script::bindings::property]]
    const sp<Boolean>& discarded() const;
//  [[script::bindings::property]]
    const sp<Boolean>& visible() const;

    sp<RenderObject> toRenderObject() const;

    wchar_t character() const;

//  [[script::bindings::property]]
    const V2& occupySize() const;
//  [[script::bindings::property]]
    const V2& contentSize() const;

    void setSize(const V2& occupySize, const V2& contentSize);

private:
    sp<Integer> _type;
    sp<Font> _font;
    sp<Vec3> _position;
    sp<Transform> _transform;
    sp<Varyings> _varyings;
    sp<Boolean> _visible;
    sp<Boolean> _discarded;

    V2 _occupy_size;
    V2 _content_size;

    friend class GlyphMaker;
};

}
