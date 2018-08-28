#include "renderer/impl/render_command/draw_elements_instanced.h"

#include "renderer/base/gl_drawing_context.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/graphics_context.h"
#include "renderer/util/gl_debug.h"

namespace ark {

DrawElementsInstanced::DrawElementsInstanced(GLDrawingContext context, const sp<GLShader>& shader, int32_t count)
    : _context(std::move(context)), _shader(shader), _count(count)
{
}

void DrawElementsInstanced::draw(GraphicsContext& graphicsContext)
{
    _shader->use(graphicsContext);
    _shader->bindUniforms(graphicsContext);

    _shader->glUpdateVPMatrix(graphicsContext, _context._camera._vp);

    _context.preDraw(graphicsContext, _shader);
    glDrawElementsInstanced(_context._mode, _context._count, GLIndexType, nullptr, _count);
    _context.postDraw(graphicsContext);
}

}
