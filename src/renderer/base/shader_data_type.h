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
        TYPE_UINTEGER,
        TYPE_COUNT
    };

public:
    ShaderDataType();
    ShaderDataType(ComponentType componentType, uint32_t numberOfComponents);

    ComponentType componentType() const;

    uint32_t size() const;
    uint32_t sizeOfComponent() const;
    uint32_t numberOfComponent() const;

private:
    ComponentType _component_type;
    uint32_t _number_of_components;
};

}
