#ifndef ARK_RENDERER_BASE_GL_DRAWING_CONTEXT_H_
#define ARK_RENDERER_BASE_GL_DRAWING_CONTEXT_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/color.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLDrawingContext {
public:
    GLDrawingContext(const sp<GLSnippet>& snippet, const GLBuffer::Snapshot& arrayBuffer, const GLBuffer& indexBuffer, GLenum mode);
    GLDrawingContext(const GLDrawingContext& other) = default;
    GLDrawingContext(GLDrawingContext&& other) = default;

    void preDraw(GraphicsContext& graphicsContext, const GLShader& shader);
    void postDraw(GraphicsContext& graphicsContext);

    sp<GLSnippet> _snippet;

    GLBuffer::Snapshot _array_buffer;
    GLBuffer _index_buffer;

    GLenum _mode;

};

}

#endif
