#pragma once

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API MaterialMap {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_BASE_COLOR,
        TYPE_NORMAL,
        TYPE_ROUGHNESS,
        TYPE_METALLIC,
        TYPE_SPECULAR,
        TYPE_EMISSION,
        TYPE_LENGTH
    };

    MaterialMap(sp<Vec4> color = nullptr, sp<Numeric> value = nullptr, sp<Bitmap> bitmap = nullptr);

//  [[script::bindings::property]]
    const sp<Vec4>& color() const;
//  [[script::bindings::property]]
    void setColor(sp<Vec4> color);

//  [[script::bindings::property]]
    const sp<Numeric>& value() const;
//  [[script::bindings::property]]
    void setValue(sp<Numeric> value);

//  [[script::bindings::property]]
    const sp<Bitmap>& bitmap() const;
//  [[script::bindings::property]]
    void setBitmap(sp<Bitmap> bitmap);

private:
    sp<Vec4> _color;
    sp<Numeric> _value;
    sp<Bitmap> _bitmap;
};

}
