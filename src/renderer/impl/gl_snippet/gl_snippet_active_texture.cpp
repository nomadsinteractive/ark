#include "renderer/impl/gl_snippet/gl_snippet_active_texture.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"

namespace ark {

GLSnippetActiveTexture::GLSnippetActiveTexture(const sp<GLTexture>& texture, uint32_t id)
    : _texture(texture), _id(id)
{
}

void GLSnippetActiveTexture::preDraw(GraphicsContext& /*graphicsContext*/, const GLShader& shader, const GLDrawingContext& /*context*/)
{
    _texture->active(shader.program(), _id);
}

}
