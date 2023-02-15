#ifndef ARK_RENDERER_BASE_DRAW_CONTEXT_H_
#define ARK_RENDERER_BASE_DRAW_CONTEXT_H_

#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/types/optional.h"

#include "graphics/base/render_layer_snapshot.h"
#include "graphics/base/rect.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_context.h"

namespace ark {

class ARK_API DrawingContext : public PipelineContext {
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

    struct ARK_API ParamDrawElements : public Param<ParamDrawElements> {
        ParamDrawElements(uint32_t start, uint32_t count);

        uint32_t _count;
        uint32_t _start;
    };

    struct ARK_API ParamDrawElementsInstanced : public Param<ParamDrawElementsInstanced> {
        ParamDrawElementsInstanced(uint32_t start, uint32_t count, int32_t instanceCount, std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots);

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

    struct ARK_API ParamDrawMultiElementsIndirect : public Param<ParamDrawMultiElementsIndirect> {
        ParamDrawMultiElementsIndirect(std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots, Buffer::Snapshot indirectCmds, uint32_t drawCount);

        std::vector<std::pair<uint32_t, Buffer::Snapshot>> _instanced_array_snapshots;
        Buffer::Snapshot _indirect_cmds;
        uint32_t _draw_count;
    };

    union ARK_API Parameters {
        Parameters();
        Parameters(Parameters&& other);
        Parameters(const Parameters& other);
        ~Parameters();

        Parameters(const ParamDrawElements& drawElements);
        Parameters(ParamDrawElementsInstanced drawElementsInstanced);
        Parameters(ParamDrawMultiElementsIndirect drawMultiElementsIndirect);

        Parameters& operator =(const Parameters& other);
        Parameters& operator =(Parameters&& other);

        ParamDrawElements _draw_elements;
        ParamDrawElementsInstanced _draw_elements_instanced;
        ParamDrawMultiElementsIndirect _draw_multi_elements_indirect;

        template<typename T, typename U> static void assign(T& asignee, U&& obj) {
            if(asignee.isActive())
                asignee = std::forward<U>(obj);
            else
                new(&asignee) T(std::forward<U>(obj));
        }
    };

public:
    DrawingContext(sp<ShaderBindings> shaderBindings, sp<ByType> attachments, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssbos);
    DrawingContext(sp<ShaderBindings> shaderBindings, sp<ByType> attachments, std::vector<RenderLayerSnapshot::UBOSnapshot> ubo, std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssbos, Buffer::Snapshot vertexBuffer, Buffer::Snapshot indexBuffer, Parameters parameters);
    DEFAULT_COPY_AND_ASSIGN(DrawingContext);

    sp<RenderCommand> toRenderCommand(const RenderRequest& renderRequest);
    sp<RenderCommand> toBindCommand();

    void upload(GraphicsContext& graphicsContext);

    sp<ByType> _attachments;

    Buffer::Snapshot _vertices;
    Buffer::Snapshot _indices;

    Optional<Rect> _scissor;
    Parameters _parameters;

};

}

#endif
