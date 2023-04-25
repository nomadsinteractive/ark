#pragma once

#include <vector>
#include <variant>

#include "core/base/api.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class ARK_API DrawingParams {
public:
    struct DrawElements {
        uint32_t _start;
    };

    struct DrawElementsInstanced {
        uint32_t _start;
        uint32_t _count;
        std::vector<std::pair<uint32_t, Buffer::Snapshot>> _divided_buffer_snapshots;
    };

    struct DrawElementsIndirectCommand {
        uint32_t  _count;
        uint32_t  _instance_count;
        uint32_t  _first_index;
        uint32_t  _base_vertex;
        uint32_t  _base_instance;
    };

    struct DrawMultiElementsIndirect {
        std::vector<std::pair<uint32_t, Buffer::Snapshot>> _divided_buffer_snapshots;
        Buffer::Snapshot _indirect_cmds;
        uint32_t _indirect_cmd_count;
    };

public:
    DrawingParams() = default;
    DrawingParams(DrawElements drawElements);
    DrawingParams(DrawElementsInstanced drawElementsInstanced);
    DrawingParams(DrawMultiElementsIndirect drawMultiElementsIndirect);

    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(DrawingParams);

    const DrawElements& drawElements() const;
    const DrawElementsInstanced& drawElementsInstanced() const;
    const DrawMultiElementsIndirect& drawMultiElementsIndirect() const;

private:
    std::variant<DrawElements, DrawElementsInstanced, DrawMultiElementsIndirect> _params;

};

}
