#include "graphics/base/material_texture.h"

#include "core/base/enum.h"
#include "core/base/string.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/string_convert.h"

#include "graphics/base/color.h"

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
    constexpr enums::LookupTable<MaterialTexture::Type, 5> table = {{
        {"base_color", MaterialTexture::TYPE_BASE_COLOR},
        {"normal", MaterialTexture::TYPE_NORMAL},
        {"roughness", MaterialTexture::TYPE_ROUGHNESS},
        {"metallic", MaterialTexture::TYPE_METALLIC},
        {"specular", MaterialTexture::TYPE_SPECULAR}
    }};
    return enums::lookup(table, str);
}

}
