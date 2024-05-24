#include "graphics/base/material_texture.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/string_convert.h"

namespace ark {

MaterialTexture::MaterialTexture(sp<Vec4> color, sp<Bitmap> bitmap)
    : _color(std::move(color)), _bitmap_wrapper(sp<VariableWrapper<sp<Bitmap>>>::make(std::move(bitmap)))
{
}

const sp<Vec4>& MaterialTexture::color() const
{
    return _color;
}

void MaterialTexture::setColor(sp<Vec4> color)
{
    _color = std::move(color);
}

sp<Bitmap> MaterialTexture::bitmap() const
{
    return _bitmap_wrapper->val();
}

const sp<VariableWrapper<sp<Bitmap>>>& MaterialTexture::bitmapWrapper() const
{
    return _bitmap_wrapper;
}

template<> ARK_API MaterialTexture::Type StringConvert::eval<MaterialTexture::Type>(const String& str)
{
    if(str == "base_color")
        return MaterialTexture::TYPE_BASE_COLOR;
    if(str == "normal")
        return MaterialTexture::TYPE_NORMAL;
    if(str == "roughness")
        return MaterialTexture::TYPE_ROUGHNESS;
    if(str == "metallic")
        return MaterialTexture::TYPE_METALLIC;
    DCHECK(str == "specular", "Unknow texture-type: %s, possible values are [base_color, normal, roughness, metallic, specular]");
    return MaterialTexture::TYPE_SPECULAR;
}

}
