#ifndef ARK_GRAPHICS_BASE_GLYPH_H_
#define ARK_GRAPHICS_BASE_GLYPH_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Glyph {
public:
//  [[script::bindings::auto]]
    Glyph(const sp<Integer>& type, const sp<Vec3>& position = nullptr, const sp<Transform>& transform = nullptr, const sp<Varyings>& varyings = nullptr, const sp<Visibility>& visible = nullptr, const sp<Disposed>& disposed = nullptr);

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
    const sp<Disposed>& disposed() const;
//  [[script::bindings::property]]
    const sp<Visibility>& visible() const;

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
    sp<Visibility> _visible;

    sp<Disposed> _disposed;

    wchar_t _character;
    V3 _layout_position;
    V2 _layout_size;
};

}

#endif
