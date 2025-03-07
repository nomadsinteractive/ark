#include "core/base/enum.h"

#include "core/base/string.h"
#include "core/types/box.h"
#include "core/util/string_convert.h"

namespace ark {

template<> Enum::DrawMode StringConvert::eval<Enum::DrawMode>(const String& str)
{
    if(str == "lines")
        return Enum::DRAW_MODE_LINES;
    if(str == "points")
        return Enum::DRAW_MODE_POINTS;
    if(str == "triangles")
        return Enum::DRAW_MODE_TRIANGLES;
    DCHECK(str == "triangle_strip", "Unknow RenderMode: %s, possible values are: [lines, points, triangles, triangle_strip]", str.c_str());
    if(str == "triangle_strip")
        return Enum::DRAW_MODE_TRIANGLE_STRIP;
    return Enum::DRAW_MODE_NONE;
}

template<> Enum::DrawProcedure StringConvert::eval<Enum::DrawProcedure>(const String& str)
{
    constexpr Enum::LookupTable<StringView, Enum::DrawProcedure, 4> table = {{
        {"auto", Enum::DRAW_PROCEDURE_AUTO},
        {"draw_arrays", Enum::DRAW_PROCEDURE_DRAW_ARRAYS},
        {"draw_elements", Enum::DRAW_PROCEDURE_DRAW_ELEMENTS},
        {"draw_instanced", Enum::DRAW_PROCEDURE_DRAW_INSTANCED}
    }};
    return Enum::lookup<Enum::DrawProcedure, 4>(table, str);
}

uint32_t Enum::__index__(const Box& self)
{
    return self.toInteger();
}

int32_t Enum::__and__(const Box& lvalue, const Box& rvalue)
{
    return lvalue.toInteger() & rvalue.toInteger();
}

int32_t Enum::__or__(const Box& lvalue, const Box& rvalue)
{
    return lvalue.toInteger() | rvalue.toInteger();
}

}
