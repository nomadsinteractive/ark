#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Glyph {
public:
//  [[script::bindings::auto]]
    Glyph(sp<Integer> type, sp<Vec3> position = nullptr, sp<Transform> transform = nullptr, sp<Varyings> varyings = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr);

//  [[script::bindings::property]]
    const sp<Integer>& type() const;
//  [[script::bindings::property]]
    const sp<Vec3>& position() const;
//  [[script::bindings::property]]
    const sp<Transform>& transform() const;
//  [[script::bindings::property]]
    const sp<Varyings>& varyings() const;
//  [[script::bindings::property]]
    void setVaryings(const sp<Varyings>& varyings);
//  [[script::bindings::property]]
    const sp<Boolean>& disposed() const;
//  [[script::bindings::property]]
    const sp<Boolean>& visible() const;

    sp<RenderObject> toRenderObject() const;
    sp<Vec3> toRenderObjectPosition() const;

    wchar_t character() const;
    void setCharacter(wchar_t character);

    const V3& layoutPosition() const;
    void setLayoutPosition(const V3& layoutPosition);

    const V2& layoutSize() const;
    void setLayoutSize(const V2& layoutSize);

private:
    sp<Integer> _type;
    sp<Vec3> _position;
    sp<Transform> _transform;
    sp<Varyings> _varyings;
    sp<Boolean> _visible;
    sp<Boolean> _disposed;

    wchar_t _character;
    V3 _layout_position;
    V2 _layout_size;
};

}
