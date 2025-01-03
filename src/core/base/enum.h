#pragma once

#include "core/base/api.h"

namespace ark {

class ARK_API Enum {
public:
//  [[script::bindings::enumeration]]
    enum BuildType {
        BUILD_TYPE_UNDEFINED,
        BUILD_TYPE_DEBUG,
        BUILD_TYPE_RELEASE,
        BUILD_TYPE_RELEASE_WITH_DEBUG_INFO,
        BUILD_TYPE_PUBLISHED = 100
    };

//  [[script::bindings::enumeration]]
    enum RenderMode {
        RENDER_MODE_NONE = -1,
        RENDER_MODE_LINES,
        RENDER_MODE_POINTS,
        RENDER_MODE_TRIANGLES,
        RENDER_MODE_TRIANGLE_STRIP,
        RENDER_MODE_COUNT,
    };

//  [[script::bindings::enumeration]]
    enum DrawProcedure {
        DRAW_PROCEDURE_AUTO,
        DRAW_PROCEDURE_DRAW_ARRAYS,
        DRAW_PROCEDURE_DRAW_ELEMENTS,
        DRAW_PROCEDURE_DRAW_INSTANCED,
        DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT
    };

    enum ShaderStageBit {
        SHADER_STAGE_BIT_NONE = -1,
        SHADER_STAGE_BIT_VERTEX,
#ifndef ANDROID
        SHADER_STAGE_BIT_TESSELLATION_CTRL,
        SHADER_STAGE_BIT_TESSELLATION_EVAL,
        SHADER_STAGE_BIT_GEOMETRY,
#endif
        SHADER_STAGE_BIT_FRAGMENT,
        SHADER_STAGE_BIT_COMPUTE,
        SHADER_STAGE_BIT_COUNT
    };

public:

//  [[script::bindings::operator(index)]]
    static uint32_t __index__(const Box& self);
//[[script::bindings::operator(&)]]
    static int32_t __and__(const Box& lvalue, const Box& rvalue);
//[[script::bindings::operator(|)]]
    static int32_t __or__(const Box& lvalue, const Box& rvalue);
};

}
