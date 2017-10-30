#include "renderer/base/gl_snippet_context.h"

namespace ark {

GLSnippetContext::GLSnippetContext(const sp<GLTexture>& texture, const GLBuffer::Snapshot& arrayBuffer, const GLBuffer& indexBuffer, GLenum mode)
    : _texture(texture), _array_buffer(arrayBuffer), _index_buffer(indexBuffer), _mode(mode)
{
}

GLSnippetContext::GLSnippetContext(const GLSnippetContext& other)
    : _texture(other._texture), _array_buffer(other._array_buffer), _index_buffer(other._index_buffer), _mode(other._mode)
{
}

GLSnippetContext::GLSnippetContext(GLSnippetContext&& other)
    : _texture(std::move(other._texture)), _array_buffer(std::move(other._array_buffer)), _index_buffer(std::move(other._index_buffer)), _mode(other._mode)
{
}

}
