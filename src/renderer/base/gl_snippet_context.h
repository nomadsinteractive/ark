#ifndef ARK_RENDERER_BASE_GL_SNIPPET_CONTEXT_H_
#define ARK_RENDERER_BASE_GL_SNIPPET_CONTEXT_H_

#include "core/types/shared_ptr.h"

#include "graphics/base/color.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

#include "platform/gl/gl.h"

namespace ark {

class GLSnippetContext {
public:
    GLSnippetContext(const sp<GLTexture>& texture, const GLBuffer::Snapshot& arrayBuffer, const GLBuffer& indexBuffer, GLenum mode);
    GLSnippetContext(const GLSnippetContext& other);
    GLSnippetContext(GLSnippetContext&& other);

    sp<GLTexture> _texture;

    GLBuffer::Snapshot _array_buffer;
    GLBuffer _index_buffer;

    GLenum _mode;

};

}

#endif
