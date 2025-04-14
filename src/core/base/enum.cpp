#include "core/base/enum.h"

#include "core/base/string.h"
#include "core/types/box.h"
#include "core/util/string_convert.h"

namespace ark {

template<> enums::DrawMode StringConvert::eval<enums::DrawMode>(const String& str)
{
    if(str == "lines")
        return enums::DRAW_MODE_LINES;
    if(str == "points")
        return enums::DRAW_MODE_POINTS;
    if(str == "triangles")
        return enums::DRAW_MODE_TRIANGLES;
    DCHECK(str == "triangle_strip", "Unknow RenderMode: %s, possible values are: [lines, points, triangles, triangle_strip]", str.c_str());
    if(str == "triangle_strip")
        return enums::DRAW_MODE_TRIANGLE_STRIP;
    return enums::DRAW_MODE_NONE;
}

template<> enums::DrawProcedure StringConvert::eval<enums::DrawProcedure>(const String& str)
{
    constexpr enums::LookupTable<enums::DrawProcedure, 4> table = {{
        {"auto", enums::DRAW_PROCEDURE_AUTO},
        {"draw_arrays", enums::DRAW_PROCEDURE_DRAW_ARRAYS},
        {"draw_elements", enums::DRAW_PROCEDURE_DRAW_ELEMENTS},
        {"draw_instanced", enums::DRAW_PROCEDURE_DRAW_INSTANCED}
    }};
    return enums::lookup(table, str);
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
