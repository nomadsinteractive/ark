#include "renderer/impl/render_command/draw_elements.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/graphics_context.h"
#include "renderer/inf/gl_snippet.h"

namespace ark {

DrawElements::DrawElements(const GLSnippetContext& context, const sp<GLShader>& shader, const sp<GLSnippet>& procedure)
    : _context(context), _shader(shader), _snippet(procedure)
{
}

void DrawElements::draw(GraphicsContext& graphicsContext)
{
    _shader->use(graphicsContext);
    _shader->bindUniforms(graphicsContext);

    if(_context._texture)
        _context._texture->active(_shader->program(), static_cast<uint32_t>(GL_TEXTURE0));
    graphicsContext.glUpdateMVPMatrix();

    _snippet->preDraw(graphicsContext, _shader, _context);
    glDrawElements(_context._mode, _context._index_buffer.length<uint16_t>(), GL_UNSIGNED_SHORT, 0);
    _snippet->postDraw(graphicsContext);
}

}
