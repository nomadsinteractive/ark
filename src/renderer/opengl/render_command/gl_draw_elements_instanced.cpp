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
    _shader->use(graphicsContext);
    _shader->bindUniforms(graphicsContext);

    GLPipeline* pipeline = static_cast<GLPipeline*>(_shader->pipeline().get());
    pipeline->glUpdateMatrix(graphicsContext, "u_VP", _context._camera._vp);

    _context.preDraw(graphicsContext, _shader);
    glDrawElementsInstanced(_mode, _context._count, GLIndexType, nullptr, _count);
    _context.postDraw(graphicsContext);
}

}
}
