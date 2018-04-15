#ifndef ARK_RENDERER_BASE_GL_DRAWING_CONTEXT_H_
#define ARK_RENDERER_BASE_GL_DRAWING_CONTEXT_H_

#include <map>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLDrawingContext {
public:
    GLDrawingContext(const sp<GLShaderBindings>& shaderBindings, const GLBuffer::Snapshot& arrayBuffer, const GLBuffer::Snapshot& indexBuffer, GLenum mode);
    GLDrawingContext(const GLDrawingContext& other) = default;
    GLDrawingContext(GLDrawingContext&& other) = default;

    void preDraw(GraphicsContext& graphicsContext, const GLShader& shader);
    void postDraw(GraphicsContext& graphicsContext);

    sp<GLShaderBindings> _shader_bindings;

    GLBuffer::Snapshot _array_buffer;
    std::map<uint32_t, GLBuffer::Snapshot> _instanced_array_buffers;
    GLBuffer::Snapshot _index_buffer;

    GLenum _mode;

};

const GLenum GLIndexType = sizeof(glindex_t) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

}

#endif
