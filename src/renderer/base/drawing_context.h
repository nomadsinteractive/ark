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
    struct Parameters {
        Parameters();
        Parameters(int32_t instanceCount, uint32_t start, uint32_t count);

        int32_t _instance_count;

        uint32_t _start;
        uint32_t _count;

//        Rect _scissor;
    };

public:
    DrawingContext(const sp<ShaderBindings>& shaderBindings, const sp<ByType>& attachments, std::vector<RenderLayer::UBOSnapshot> ubo);
    DrawingContext(const sp<ShaderBindings>& shaderBindings, const sp<ByType>& attachments, std::vector<RenderLayer::UBOSnapshot> ubo, Buffer::Snapshot vertexBuffer, Buffer::Snapshot indexBuffer, int32_t instanceCount);
    DrawingContext(const sp<ShaderBindings>& shaderBindings, const sp<ByType>& attachments, std::vector<RenderLayer::UBOSnapshot> ubo, Buffer::Snapshot vertexBuffer, Buffer::Snapshot indexBuffer, int32_t instanceCount, uint32_t start, uint32_t count);
    DEFAULT_COPY_AND_ASSIGN(DrawingContext);

    sp<RenderCommand> toRenderCommand();

    sp<ShaderBindings> _shader_bindings;
    sp<ByType> _attachments;

    std::vector<RenderLayer::UBOSnapshot> _ubos;
    Buffer::Snapshot _vertex_buffer;
    Buffer::Snapshot _index_buffer;

    std::vector<std::pair<uint32_t, Buffer::Snapshot>> _instanced_array_snapshots;

    Rect _scissor;
    Parameters _parameters;

private:
    void upload(GraphicsContext& graphicsContext);

    void preDraw(GraphicsContext& graphicsContext);
    void postDraw(GraphicsContext& graphicsContext);


private:
    class RenderCommandBind;
    class RenderCommandDraw;

    friend class RenderCommandBind;
    friend class RenderCommandDraw;
};

}

#endif
