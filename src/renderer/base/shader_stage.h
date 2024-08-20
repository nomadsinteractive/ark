#pragma once

#include <functional>

#include "core/base/api.h"
#include "core/base/bit_set.h"

namespace ark {

class ARK_API ShaderStage {
public:
    enum Set {
        SHADER_STAGE_NONE = -1,
        SHADER_STAGE_VERTEX,
#ifndef ANDROID
        SHADER_STAGE_TESSELLATION_CTRL,
        SHADER_STAGE_TESSELLATION_EVAL,
        SHADER_STAGE_GEOMETRY,
#endif
        SHADER_STAGE_FRAGMENT,
        SHADER_STAGE_COMPUTE,
        SHADER_STAGE_COUNT
    };

    void add(Set stage);
    bool has(Set stage) const;

    template<typename T> T toFlags(const std::function<T(Set)>& converter) const {
        T flags = static_cast<T>(0);
        for(size_t i = 0; i < SHADER_STAGE_COUNT; ++i)
            if(_stages.has(static_cast<Set>(1 << i)))
                flags = static_cast<T>(flags | converter(static_cast<Set>(i)));
        return flags;
    }

private:
    BitSet<Set> _stages;
};

}
