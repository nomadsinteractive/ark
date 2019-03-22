#ifndef ARK_RENDERER_BASE_DRAWING_CONTEXT_H_
#define ARK_RENDERER_BASE_DRAWING_CONTEXT_H_

#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/matrix.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class ARK_API DrawingContext {
public:
    DrawingContext(const sp<Shader>& shader, const sp<ShaderBindings>& shaderBindings, std::vector<RenderLayer::UBOSnapshot> ubo);
    DrawingContext(const sp<Shader>& shader, const sp<ShaderBindings>& shaderBindings, std::vector<RenderLayer::UBOSnapshot> ubo, const Buffer::Snapshot& vertexBuffer, const Buffer::Snapshot& indexBuffer, int32_t instanceCount);
    DEFAULT_COPY_AND_ASSIGN(DrawingContext);

    sp<RenderCommand> toRenderCommand(ObjectPool& objectPool);

    sp<Shader> _shader;
    sp<ShaderBindings> _shader_bindings;

    std::vector<RenderLayer::UBOSnapshot> _ubos;
    Buffer::Snapshot _vertex_buffer;
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> _instanced_array_snapshots;

    Buffer::Snapshot _index_buffer;

    int32_t _count;
    int32_t _instance_count;

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
