#include "graphics/base/material_map.h"

#include "core/base/enum.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/util/string_convert.h"

namespace ark {

MaterialMap::MaterialMap(sp<Vec4> color, sp<Bitmap> bitmap)
    : _color(std::move(color)), _bitmap(std::move(bitmap))
{
}

const sp<Vec4>& MaterialMap::color() const
{
    return _color;
}

void MaterialMap::setColor(sp<Vec4> color)
{
    _color = std::move(color);
}

const sp<Bitmap>& MaterialMap::bitmap() const
{
    return _bitmap;
}

void MaterialMap::setBitmap(sp<Bitmap> bitmap)
{
    _bitmap = std::move(bitmap);
}

template<> ARK_API MaterialMap::Type StringConvert::eval<MaterialMap::Type>(const String& str)
{
    constexpr enums::LookupTable<MaterialMap::Type, 5> table = {{
        {"base_color", MaterialMap::TYPE_BASE_COLOR},
        {"normal", MaterialMap::TYPE_NORMAL},
        {"roughness", MaterialMap::TYPE_ROUGHNESS},
        {"metallic", MaterialMap::TYPE_METALLIC},
        {"specular", MaterialMap::TYPE_SPECULAR}
    }};
    return enums::lookup(table, str);
}

}
