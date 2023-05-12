#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API ShaderDataType {
public:
    enum ComponentType {
        TYPE_NONE,
        TYPE_BYTE,
        TYPE_FLOAT,
        TYPE_INTEGER,
        TYPE_SHORT,
        TYPE_UBYTE,
        TYPE_USHORT,
        TYPE_COUNT
    };

private:
    ComponentType _component_type;

    uint32_t _component_size;
    uint32_t _array_length;
};

}
