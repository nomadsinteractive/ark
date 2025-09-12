#include "graphics/base/material_texture.h"

#include "core/base/enum.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/util/string_convert.h"

namespace ark {

namespace {

sp<Variable<sp<Bitmap>>> toBitmapProvider(sp<Bitmap> bitmap)
{
    if(bitmap)
        return sp<Variable<sp<Bitmap>>>::make<Variable<sp<Bitmap>>::Const>(std::move(bitmap));
    return nullptr;
}

}

MaterialTexture::MaterialTexture(sp<Vec4> color, sp<Bitmap> bitmap)
    : _color(std::move(color)), _bitmap_provider(toBitmapProvider(std::move(bitmap)))
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
    return _bitmap_provider ? _bitmap_provider->val() : sp<Bitmap>();
}

void MaterialTexture::setBitmap(sp<Bitmap> bitmap)
{
    _bitmap_provider = toBitmapProvider(std::move(bitmap));
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
