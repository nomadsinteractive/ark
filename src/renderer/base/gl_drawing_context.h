#ifndef ARK_RENDERER_BASE_GL_DRAWING_CONTEXT_H_
#define ARK_RENDERER_BASE_GL_DRAWING_CONTEXT_H_

#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/camera.h"
#include "graphics/base/matrix.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLDrawingContext {
public:
    GLDrawingContext(const sp<GLShaderBindings>& shaderBindings, const Camera::Snapshot& camera, const GLBuffer::Snapshot& arrayBuffer, const GLBuffer::Snapshot& indexBuffer, GLenum mode);
    DEFAULT_COPY_AND_ASSIGN(GLDrawingContext);

    void preDraw(GraphicsContext& graphicsContext, const GLShader& shader);
    void postDraw(GraphicsContext& graphicsContext);

    sp<GLShaderBindings> _shader_bindings;

    Camera::Snapshot _camera;
    GLBuffer::Snapshot _array_buffer;
    std::vector<std::pair<uint32_t, GLBuffer::Snapshot>> _instanced_array_snapshots;

    GLBuffer::Snapshot _index_buffer;

    GLenum _mode;
    GLsizei _count;

};

const GLenum GLIndexType = sizeof(glindex_t) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

}

#endif
