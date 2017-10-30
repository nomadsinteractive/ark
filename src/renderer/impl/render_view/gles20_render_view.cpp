#include "renderer/impl/render_view/gles20_render_view.h"

#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/inf/render_command.h"
#include "graphics/base/viewport.h"

#include "renderer/base/graphics_context.h"
#include "renderer/util/gl_debug.h"

#include "platform/gl/gl.h"

namespace ark {

uint32_t g_GLViewportWidth = 0;
uint32_t g_GLViewportHeight = 0;

GLES20RenderView::GLES20RenderView(const sp<GLResourceManager>& glResources, const Viewport& viewport)
    : _gl_resource_manager(glResources), _graphics_context(new GraphicsContext(glResources)), _viewport(viewport)
{
}

GLES20RenderView::~GLES20RenderView()
{
}

void GLES20RenderView::onSurfaceCreated()
{
    GLDebug::glPrintString("GL Version:", GL_VERSION);
    GLDebug::glPrintInteger("Max Uniform Components", GL_MAX_VERTEX_UNIFORM_COMPONENTS);
    GLDebug::glPrintInteger("Max Uniform Vectors", GL_MAX_VERTEX_UNIFORM_VECTORS);
}

void GLES20RenderView::onSurfaceChanged(uint32_t width, uint32_t height)
{
    _graphics_context.reset(new GraphicsContext(_gl_resource_manager));
    initialize(width, height);
}

void GLES20RenderView::onRenderFrame(const sp<RenderCommand>& renderCommand)
{
    _graphics_context->onDrawFrame();
    renderCommand->draw(_graphics_context);
}

void GLES20RenderView::initialize(uint32_t width, uint32_t height)
{
    LOGD("Width: %d, Height: %d, Viewport (%.1f, %.1f, %.1f, %.1f)", width, height, _viewport.left(), _viewport.top(), _viewport.right(), _viewport.bottom());

    g_GLViewportWidth = width;
    g_GLViewportHeight = height;

    glViewport(0, 0, width, height);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);

    _graphics_context->glOrtho(_viewport.left(), _viewport.right(), _viewport.top(), _viewport.bottom(), _viewport.near(), _viewport.far());
    _graphics_context->onSurfaceReady();
}

}
