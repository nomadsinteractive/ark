#include "renderer/impl/render_command/draw_elements_instanced.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/graphics_context.h"

namespace ark {

DrawElementsInstanced::DrawElementsInstanced(const GLDrawingContext& context, const sp<GLShader>& shader, int32_t count)
    : _context(context), _shader(shader), _count(count)
{
}

void DrawElementsInstanced::draw(GraphicsContext& graphicsContext)
{
    _shader->use(graphicsContext);
    _shader->bindUniforms(graphicsContext);

    graphicsContext.glUpdateMVPMatrix();

    _context.preDraw(graphicsContext, _shader);
    const uint16_t indices[] = {0, 2, 1, 2, 3, 1};
    glDrawElementsInstanced(_context._mode, 6, GL_UNSIGNED_SHORT, indices, _count);
    _context.postDraw(graphicsContext);
}

}
