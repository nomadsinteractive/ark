#include "core/base/enum.h"

#include "core/base/string.h"
#include "core/base/enum_map.h"
#include "core/util/string_convert.h"

namespace ark {

Enum::Enum(int32_t value)
    : _value(value)
{
}

template<> Enum::RenderMode StringConvert::to<String, Enum::RenderMode>(const String& str)
{
    if(str == "lines")
        return Enum::RENDER_MODE_LINES;
    if(str == "points")
        return Enum::RENDER_MODE_POINTS;
    if(str == "triangles")
        return Enum::RENDER_MODE_TRIANGLES;
    DCHECK(str == "triangle_strip", "Unknow RenderMode: %s, possible values are: [lines, points, triangles, triangle_strip]", str.c_str());
    if(str == "triangle_strip")
        return Enum::RENDER_MODE_TRIANGLE_STRIP;
    return Enum::RENDER_MODE_NONE;
}

template<> void EnumMap<Enum::DrawProcedure>::initialize(std::map<String, Enum::DrawProcedure>& enums)
{
    enums["auto"] = Enum::DRAW_PROCEDURE_AUTO;
    enums["draw_arrays"] = Enum::DRAW_PROCEDURE_DRAW_ARRAYS;
    enums["draw_elements"] = Enum::DRAW_PROCEDURE_DRAW_ELEMENTS;
    enums["draw_instanced"] = Enum::DRAW_PROCEDURE_DRAW_INSTANCED;
}

}
