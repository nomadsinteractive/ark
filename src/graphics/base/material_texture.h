#ifndef ARK_GRAPHICS_BASE_MATERIAL_TEXTURE_H_
#define ARK_GRAPHICS_BASE_MATERIAL_TEXTURE_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API MaterialTexture {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_BASE_COLOR,
        TYPE_NORMAL,
        TYPE_ROUGHNESS,
        TYPE_METALLIC,
        TYPE_SPECULAR,
        TYPE_LENGTH
    };

    MaterialTexture(sp<Vec4> color = nullptr, sp<Bitmap> bitmap = nullptr);

//  [[script::bindings::property]]
    const sp<Vec4>& color() const;
//  [[script::bindings::property]]
    void setColor(sp<Vec4> color);

//  [[script::bindings::property]]
    sp<Bitmap> bitmap() const;

    const sp<VariableWrapper<sp<Bitmap>>>& bitmapWrapper() const;

private:
    Type _type;
    sp<Vec4> _color;
    sp<VariableWrapper<sp<Bitmap>>> _bitmap_wrapper;
};

}

#endif
