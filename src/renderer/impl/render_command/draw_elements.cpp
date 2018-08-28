#include "renderer/impl/render_command/draw_elements.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/graphics_context.h"

namespace ark {

DrawElements::DrawElements(GLDrawingContext context, const sp<GLShader>& shader)
    : _context(std::move(context)), _shader(shader)
{
}

void DrawElements::draw(GraphicsContext& graphicsContext)
{
    _shader->use(graphicsContext);
    _shader->bindUniforms(graphicsContext);
    _shader->glUpdateMVPMatrix(graphicsContext, _context._camera._vp);

    _context.preDraw(graphicsContext, _shader);
    glDrawElements(_context._mode, _context._count, GLIndexType, 0);
    _context.postDraw(graphicsContext);
}

}
