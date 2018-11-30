#include "renderer/opengl/render_command/draw_elements.h"

#include "renderer/base/shader.h"
#include "renderer/base/graphics_context.h"

#include "renderer/opengl/base/gl_pipeline.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

DrawElements::DrawElements(DrawingContext context, const sp<Shader>& shader, GLenum mode)
    : _context(std::move(context)), _shader(shader), _mode(mode)
{
}

void DrawElements::draw(GraphicsContext& graphicsContext)
{
    _shader->use(graphicsContext);
    _shader->bindUniforms(graphicsContext);

    GLPipeline* pipeline = static_cast<GLPipeline*>(_shader->pipeline().get());
    pipeline->glUpdateMatrix(graphicsContext, "u_MVP", _context._camera._vp);

    _context.preDraw(graphicsContext, _shader);
    glDrawElements(_mode, _context._count, GLIndexType, nullptr);
    _context.postDraw(graphicsContext);
}

}
}
