#include "renderer/opengl/render_command/gl_draw_elements.h"

#include "renderer/base/shader.h"
#include "renderer/base/graphics_context.h"

#include "renderer/opengl/base/gl_pipeline.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

GLDrawElements::GLDrawElements(DrawingContext context, const sp<Shader>& shader, GLenum mode)
    : _context(std::move(context)), _shader(shader), _mode(mode)
{
}

void GLDrawElements::draw(GraphicsContext& graphicsContext)
{
    _shader->active(graphicsContext, _context);
    _context.preDraw(graphicsContext);
    glDrawElements(_mode, _context._count, GLIndexType, nullptr);
    _context.postDraw(graphicsContext);
}

}
}
