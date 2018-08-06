#include "renderer/impl/gl_snippet/gl_snippet_active_cubemap.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_cubemap.h"

namespace ark {

GLSnippetActiveCubemap::GLSnippetActiveCubemap(const sp<GLCubemap>& cubemap, uint32_t id)
    : _cubemap(cubemap), _id(id)
{
}

void GLSnippetActiveCubemap::preDraw(GraphicsContext& /*graphicsContext*/, const GLShader& shader, const GLDrawingContext& /*context*/)
{
    _cubemap->active(shader.program(), _id);
}

}
