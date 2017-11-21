#include "renderer/base/gl_snippet_context.h"

namespace ark {

GLSnippetContext::GLSnippetContext(const sp<GLTexture>& texture, const GLBuffer::Snapshot& arrayBuffer, const GLBuffer& indexBuffer, GLenum mode)
    : _texture(texture), _array_buffer(arrayBuffer), _index_buffer(indexBuffer), _mode(mode)
{
}

}
