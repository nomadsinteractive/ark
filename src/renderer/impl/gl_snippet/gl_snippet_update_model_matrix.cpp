#include "renderer/impl/gl_snippet/gl_snippet_update_model_matrix.h"

#include "renderer/base/graphics_context.h"

namespace ark {

void GLSnippetUpdateModelMatrix::preDraw(GraphicsContext& graphicsContext, const GLShader& /*shader*/, const GLDrawingContext& /*context*/)
{
    graphicsContext.glUpdateModelMatrix();
}

}
