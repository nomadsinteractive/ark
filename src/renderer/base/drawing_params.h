#pragma once

#include <vector>

#include "core/base/api.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

union ARK_API DrawingParams {
public:

    template<typename T> struct Param {
        Param()
            : _type(Type<T>::id()) {
        }

        bool isActive() const {
            return _type == Type<T>::id();
        }

        TypeId _type;
    };

    struct ARK_API DrawElements : public Param<DrawElements> {
        DrawElements(uint32_t start = 0);

        uint32_t _start;
    };

    struct ARK_API DrawElementsInstanced : public Param<DrawElementsInstanced> {
        DrawElementsInstanced(uint32_t start, uint32_t count, std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots);

        uint32_t _count;
        uint32_t _start;

        std::vector<std::pair<uint32_t, Buffer::Snapshot>> _divided_buffer_snapshots;
    };

    struct DrawElementsIndirectCommand {
        uint32_t  _count;
        uint32_t  _instance_count;
        uint32_t  _first_index;
        uint32_t  _base_vertex;
        uint32_t  _base_instance;
    };

    struct ARK_API DrawMultiElementsIndirect : public Param<DrawMultiElementsIndirect> {
        DrawMultiElementsIndirect(std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots, Buffer::Snapshot indirectCmds, uint32_t indirectCmdCount);

        std::vector<std::pair<uint32_t, Buffer::Snapshot>> _divided_buffer_snapshots;
        Buffer::Snapshot _indirect_cmds;
        uint32_t _indirect_cmd_count;
    };

    DrawingParams();
    DrawingParams(DrawingParams&& other);
    DrawingParams(const DrawingParams& other);
    ~DrawingParams();

    DrawingParams(DrawElements drawElements);
    DrawingParams(DrawElementsInstanced drawElementsInstanced);
    DrawingParams(DrawMultiElementsIndirect drawMultiElementsIndirect);

    DrawingParams& operator =(const DrawingParams& other);
    DrawingParams& operator =(DrawingParams&& other);

    DrawElements _draw_elements;
    DrawElementsInstanced _draw_elements_instanced;
    DrawMultiElementsIndirect _draw_multi_elements_indirect;

private:

    template<typename T, typename U> static void assign(T& asignee, U&& obj) {
        if(asignee.isActive())
            asignee = std::forward<U>(obj);
        else
            new(&asignee) T(std::forward<U>(obj));
    }

};

}
