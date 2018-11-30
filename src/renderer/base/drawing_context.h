#ifndef ARK_RENDERER_BASE_DRAWING_CONTEXT_H_
#define ARK_RENDERER_BASE_DRAWING_CONTEXT_H_

#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/camera.h"
#include "graphics/base/matrix.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

namespace ark {

class ARK_API DrawingContext {
public:
    DrawingContext(const sp<ShaderBindings>& shaderBindings, const Camera::Snapshot& camera, const GLBuffer::Snapshot& arrayBuffer, const GLBuffer::Snapshot& indexBuffer);
    DEFAULT_COPY_AND_ASSIGN(DrawingContext);

    void preDraw(GraphicsContext& graphicsContext, const Shader& shader);
    void postDraw(GraphicsContext& graphicsContext);

    sp<ShaderBindings> _shader_bindings;

    Camera::Snapshot _camera;
    GLBuffer::Snapshot _array_buffer;
    std::vector<std::pair<uint32_t, GLBuffer::Snapshot>> _instanced_array_snapshots;

    GLBuffer::Snapshot _index_buffer;

    int32_t _count;

};

}

#endif
