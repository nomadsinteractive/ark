#pragma once

#include <array>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/string_buffer.h"

namespace ark {

namespace enums {

//[[script::bindings::enumeration(Enum)]]
enum BuildType {
    BUILD_TYPE_UNDEFINED,
    BUILD_TYPE_DEBUG,
    BUILD_TYPE_RELEASE,
    BUILD_TYPE_RELEASE_WITH_DEBUG_INFO,
    BUILD_TYPE_PUBLISHED = 100
};

//[[script::bindings::enumeration(Enum)]]
enum DrawMode {
    DRAW_MODE_NONE = -1,
    DRAW_MODE_LINES,
    DRAW_MODE_POINTS,
    DRAW_MODE_TRIANGLES,
    DRAW_MODE_TRIANGLE_STRIP,
    DRAW_MODE_COUNT
};

//[[script::bindings::enumeration(Enum)]]
enum DrawProcedure {
    DRAW_PROCEDURE_AUTO,
    DRAW_PROCEDURE_DRAW_ARRAYS,
    DRAW_PROCEDURE_DRAW_ELEMENTS,
    DRAW_PROCEDURE_DRAW_INSTANCED,
    DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT
};

//[[script::bindings::enumeration(Enum)]]
enum UploadStrategyBit {
    UPLOAD_STRATEGY_RELOAD = 0,
    UPLOAD_STRATEGY_ONCE = 1,
    UPLOAD_STRATEGY_ON_SURFACE_READY = 2,
    UPLOAD_STRATEGY_ON_CHANGE = 4,
    UPLOAD_STRATEGY_ON_EVERY_FRAME = 8
};

//[[script::bindings::enumeration(Enum)]]
enum UploadPriority {
    UPLOAD_PRIORITY_HIGH = 0,
    UPLOAD_PRIORITY_NORMAL,
    UPLOAD_PRIORITY_LOW,
    UPLOAD_PRIORITY_COUNT
};

//[[script::bindings::enumeration(Enum)]]
enum InsertPosition {
    INSERT_POSITION_BACK = 0,
    INSERT_POSITION_FRONT
};

//[[script::bindings::enumeration(Enum)]]
enum CoordinateSystem {
    COORDINATE_SYSTEM_DEFAULT = 0,
    COORDINATE_SYSTEM_LHS = 1,
    COORDINATE_SYSTEM_RHS = 2
};

enum NDCDepthRange {
    NDC_DEPTH_RANGE_ZERO_TO_ONE = 0,
    NDC_DEPTH_RANGE_NEGATIVE_ONE_TO_ONE = 1
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
    RENDERER_VERSION_OPENGL = 100,
    RENDERER_VERSION_OPENGL_30 = 130,
    RENDERER_VERSION_OPENGL_31 = 131,
    RENDERER_VERSION_OPENGL_32 = 132,
    RENDERER_VERSION_OPENGL_33 = 133,
    RENDERER_VERSION_OPENGL_40 = 140,
    RENDERER_VERSION_OPENGL_41 = 141,
    RENDERER_VERSION_OPENGL_42 = 142,
    RENDERER_VERSION_OPENGL_43 = 143,
    RENDERER_VERSION_OPENGL_44 = 144,
    RENDERER_VERSION_OPENGL_45 = 145,
    RENDERER_VERSION_OPENGL_46 = 146,
    RENDERER_VERSION_VULKAN = 200,
    RENDERER_VERSION_VULKAN_11 = 211,
    RENDERER_VERSION_VULKAN_12 = 212,
    RENDERER_VERSION_VULKAN_13 = 213,
    RENDERER_VERSION_DIRECT_X = 300,
    RENDERER_VERSION_DIRECT_X_11 = 311,
    RENDERER_VERSION_DIRECT_X_12 = 312,
    RENDERER_VERSION_METAL = 400,
    RENDERER_VERSION_METAL_30 = 430,
    RENDERER_VERSION_METAL_40 = 440
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

enum ShaderTypeQualifierBits {
    SHADER_TYPE_QUALIFIER_IN,
    SHADER_TYPE_QUALIFIER_OUT,
    SHADER_TYPE_QUALIFIER_FLAT,
    SHADER_TYPE_QUALIFIER_UNIFORM,
    SHADER_TYPE_QUALIFIER_READONLY,
    SHADER_TYPE_QUALIFIER_WRITEONLY
};


typedef BitSet<RenderingBackendBit> RenderingBackendSet;
typedef BitSet<ShaderStageBit, true> ShaderStageSet;
typedef BitSet<UploadStrategyBit> UploadStrategy;
typedef BitSet<ShaderTypeQualifierBits, true> ShaderTypeQualifier;


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

}

class ARK_API Enum {
public:
//  [[script::bindings::operator(index)]]
    static uint32_t __index__(const Box& self);
//  [[script::bindings::operator(&)]]
    static int32_t __and__(int32_t lhs, const Box& rhs);
//  [[script::bindings::operator(&)]]
    static int32_t __and__(const Box& lhs, int32_t rhs);
//  [[script::bindings::operator(&)]]
    static int32_t __and__(const Box& lhs, const Box& rhs);
//  [[script::bindings::operator(|)]]
    static int32_t __or__(int32_t lhs, const Box& rhs);
//  [[script::bindings::operator(|)]]
    static int32_t __or__(const Box& lhs, int32_t rhs);
//  [[script::bindings::operator(|)]]
    static int32_t __or__(const Box& lhs, const Box& rhs);
};

}
