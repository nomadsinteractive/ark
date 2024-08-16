#pragma once

#include <bitset>
#include <functional>

#include "core/base/api.h"

namespace ark {

class ARK_API ShaderStage {
public:
    enum BitSet {
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

    void add(BitSet stage);
    bool has(BitSet stage) const;

    template<typename T> T toFlags(const std::function<T(BitSet)>& converter) const {
        T flags = static_cast<T>(0);
        for(size_t i = 0; i < SHADER_STAGE_COUNT; ++i)
            if(_stages.test(i))
                flags = static_cast<T>(flags | converter(static_cast<BitSet>(i)));
        return flags;
    }

private:
    std::bitset<SHADER_STAGE_COUNT> _stages;
};

}
