#ifndef ARK_RENDERER_BASE_DRAW_CONTEXT_H_
#define ARK_RENDERER_BASE_DRAW_CONTEXT_H_

#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/rect.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class ARK_API DrawingContext {
public:
    template<typename T> struct Param {
        Param()
            : _type(Type<T>::id()) {
        }

        bool isActive() const {
            return _type == Type<T>::id();
        }

        void assign(T other) {
            if(isActive())
                (*static_cast<T*>(this)) = std::move(other);
            else
                new(static_cast<T*>(this)) T(std::move(other));
        }

        TypeId _type;
    };

    struct ARK_API ParamDrawElements : public Param<ParamDrawElements> {
        ParamDrawElements();
        ParamDrawElements(uint32_t start, uint32_t count);

        uint32_t _count;
        uint32_t _start;
    };

    struct ARK_API ParamDrawElementsInstanced : public Param<ParamDrawElementsInstanced> {
        ParamDrawElementsInstanced();
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
        ParamDrawMultiElementsIndirect();
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
        Parameters(const ParamDrawElementsInstanced& drawElementsInstanced);
        Parameters(const ParamDrawMultiElementsIndirect& drawMultiElementsIndirect);

        Parameters& operator =(const Parameters& other);

        ParamDrawElements _draw_elements;
        ParamDrawElementsInstanced _draw_elements_instanced;
        ParamDrawMultiElementsIndirect _draw_multi_elements_indirect;
    };

public:
    DrawingContext(const sp<ShaderBindings>& shaderBindings, const sp<ByType>& attachments, std::vector<RenderLayer::UBOSnapshot> ubo);
    DrawingContext(const sp<ShaderBindings>& shaderBindings, const sp<ByType>& attachments, std::vector<RenderLayer::UBOSnapshot> ubo, Buffer::Snapshot vertexBuffer, Buffer::Snapshot indexBuffer, const Parameters& parameters);
    DEFAULT_COPY_AND_ASSIGN(DrawingContext);

    sp<RenderCommand> toRenderCommand();
    sp<RenderCommand> toBindCommand();

    sp<ShaderBindings> _shader_bindings;
    sp<ByType> _attachments;

    std::vector<RenderLayer::UBOSnapshot> _ubos;
    Buffer::Snapshot _vertex_buffer;
    Buffer::Snapshot _index_buffer;

    Rect _scissor;
    Parameters _parameters;

private:
    void upload(GraphicsContext& graphicsContext);

    void preDraw(GraphicsContext& graphicsContext);
    void postDraw(GraphicsContext& graphicsContext);


private:
    class RenderCommandBind;
    class RenderCommandDraw;

};

}

#endif
