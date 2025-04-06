#pragma once

#include <array>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/string_buffer.h"

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
    enum DrawMode {
        DRAW_MODE_NONE = -1,
        DRAW_MODE_LINES,
        DRAW_MODE_POINTS,
        DRAW_MODE_TRIANGLES,
        DRAW_MODE_TRIANGLE_STRIP,
        DRAW_MODE_COUNT,
    };

//  [[script::bindings::enumeration]]
    enum DrawProcedure {
        DRAW_PROCEDURE_AUTO,
        DRAW_PROCEDURE_DRAW_ARRAYS,
        DRAW_PROCEDURE_DRAW_ELEMENTS,
        DRAW_PROCEDURE_DRAW_INSTANCED,
        DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT
    };

    enum RenderingBackendBit {
        RENDERING_BACKEND_AUTO = 0,
        RENDERING_BACKEND_BIT_OPENGL = 1 << 0,
        RENDERING_BACKEND_BIT_VULKAN = 1 << 1,
        RENDERING_BACKEND_BIT_DIRECT_X = 1 << 2,
        RENDERING_BACKEND_BIT_METAL = 1 << 3,
        RENDERING_BACKEND_ALL = RENDERING_BACKEND_BIT_OPENGL | RENDERING_BACKEND_BIT_VULKAN | RENDERING_BACKEND_BIT_DIRECT_X | RENDERING_BACKEND_BIT_METAL
    };

    enum RendererVersion {
        RENDERER_VERSION_AUTO = 0,
        RENDERER_VERSION_OPENGL = 10,
        RENDERER_VERSION_OPENGL_30 = 30,
        RENDERER_VERSION_OPENGL_31 = 31,
        RENDERER_VERSION_OPENGL_32 = 32,
        RENDERER_VERSION_OPENGL_33 = 33,
        RENDERER_VERSION_OPENGL_40 = 40,
        RENDERER_VERSION_OPENGL_41 = 41,
        RENDERER_VERSION_OPENGL_42 = 42,
        RENDERER_VERSION_OPENGL_43 = 43,
        RENDERER_VERSION_OPENGL_44 = 44,
        RENDERER_VERSION_OPENGL_45 = 45,
        RENDERER_VERSION_OPENGL_46 = 46,
        RENDERER_VERSION_VULKAN = 110,
        RENDERER_VERSION_VULKAN_11 = 111,
        RENDERER_VERSION_VULKAN_12 = 112,
        RENDERER_VERSION_VULKAN_13 = 113
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

    template<typename V, size_t N> using LookupTable = std::array<std::pair<StringView, V>, N>;
    template<typename T> constexpr static auto lookup(const T& table, const StringView key, const decltype(std::declval<T>().front().second) defaultValue) -> decltype(std::declval<T>().front().second){
        for(const auto [k, v] : table)
            if(key == k)
                return v;
        return defaultValue;
    }
    template<typename T> static auto lookup(const T& table, const StringView key) -> decltype(std::declval<T>().front().second) {
        for(const auto [k, v] : table)
            if(key == k)
                return v;

        StringBuffer sb;
        for(size_t j = 0; j < table.size(); ++j) {
            sb << table.at(j).first << '(' << static_cast<uint32_t>(table.at(j).second) << ')';
            if(j != table.size() - 1)
                sb << ", ";
        }
        FATAL("Unknow value %s, possible values are [%s]", key.data(), sb.str().c_str());
        return static_cast<decltype(std::declval<T>().front().second)>(0);
    }

//  [[script::bindings::operator(index)]]
    static uint32_t __index__(const Box& self);
//  [[script::bindings::operator(&)]]
    static int32_t __and__(const Box& lvalue, const Box& rvalue);
//  [[script::bindings::operator(|)]]
    static int32_t __or__(const Box& lvalue, const Box& rvalue);
};

}
