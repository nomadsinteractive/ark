#pragma once

#include <vector>

#include "core/base/api.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class ARK_API DrawingContextParams {
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
        DrawElements(uint32_t start, uint32_t count);

        uint32_t _count;
        uint32_t _start;
    };

    struct ARK_API DrawElementsInstanced : public Param<DrawElementsInstanced> {
        DrawElementsInstanced(uint32_t start, uint32_t count, int32_t instanceCount, std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots);

        uint32_t _count;
        uint32_t _start;

        int32_t _instance_count;

        std::vector<std::pair<uint32_t, Buffer::Snapshot>> _instanced_array_snapshots;
    };

    struct DrawElementsIndirectCommand {
        uint32_t  _count;
        uint32_t  _instance_count;
        uint32_t  _first_index;
        uint32_t  _base_vertex;
        uint32_t  _base_instance;
    };

    struct ARK_API DrawMultiElementsIndirect : public Param<DrawMultiElementsIndirect> {
        DrawMultiElementsIndirect(std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots, Buffer::Snapshot indirectCmds, uint32_t drawCount);

        std::vector<std::pair<uint32_t, Buffer::Snapshot>> _instanced_array_snapshots;
        Buffer::Snapshot _indirect_cmds;
        uint32_t _draw_count;
    };

    union ARK_API Parameters {
        Parameters();
        Parameters(Parameters&& other);
        Parameters(const Parameters& other);
        ~Parameters();

        Parameters(const DrawElements& drawElements);
        Parameters(DrawElementsInstanced drawElementsInstanced);
        Parameters(DrawMultiElementsIndirect drawMultiElementsIndirect);

        Parameters& operator =(const Parameters& other);
        Parameters& operator =(Parameters&& other);

        DrawElements _draw_elements;
        DrawElementsInstanced _draw_elements_instanced;
        DrawMultiElementsIndirect _draw_multi_elements_indirect;

        template<typename T, typename U> static void assign(T& asignee, U&& obj) {
            if(asignee.isActive())
                asignee = std::forward<U>(obj);
            else
                new(&asignee) T(std::forward<U>(obj));
        }
    };

public:
    DrawingContextParams(Buffer::Snapshot vertices, Buffer::Snapshot indices, Parameters parameters);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(DrawingContextParams);

    Buffer::Snapshot _vertices;
    Buffer::Snapshot _indices;

    Parameters _parameters;
};

}
