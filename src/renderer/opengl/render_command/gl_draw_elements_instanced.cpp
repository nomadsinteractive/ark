#include "renderer/opengl/render_command/gl_draw_elements_instanced.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/shader.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark {
namespace opengl {

GLDrawElementsInstanced::GLDrawElementsInstanced(DrawingContext context, const sp<Shader>& shader, GLenum mode, int32_t count)
    : _context(std::move(context)), _shader(shader), _mode(mode), _count(count)
{
}

void GLDrawElementsInstanced::draw(GraphicsContext& graphicsContext)
{
    _shader->active(graphicsContext, _context);
    _context.preDraw(graphicsContext);
    glDrawElementsInstanced(_mode, _context._count, GLIndexType, nullptr, _count);
    _context.postDraw(graphicsContext);
}

}
}
