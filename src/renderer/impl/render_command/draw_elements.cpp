#include "renderer/impl/render_command/draw_elements.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/graphics_context.h"

namespace ark {

DrawElements::DrawElements(const GLDrawingContext& context, const sp<GLShader>& shader)
    : _context(context), _shader(shader)
{
}

void DrawElements::draw(GraphicsContext& graphicsContext)
{
    _shader->use(graphicsContext);
    _shader->bindUniforms(graphicsContext);

    graphicsContext.glUpdateMVPMatrix();

    _context.preDraw(graphicsContext, _shader);
    glDrawElements(_context._mode, _context._index_buffer.length<uint16_t>(), GL_UNSIGNED_SHORT, 0);
    _context.postDraw(graphicsContext);
}

}
