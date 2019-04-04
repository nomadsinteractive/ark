#include "renderer/opengl/render_view/render_view_opengl.h"

#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/inf/render_command.h"
#include "graphics/base/color.h"
#include "graphics/base/viewport.h"

#include "renderer/base/graphics_context.h"
#include "renderer/opengl/util/gl_debug.h"

#include "platform/gl/gl.h"
#include "platform/platform.h"

namespace ark {

RenderViewOpenGL::RenderViewOpenGL(const sp<RenderContext>& glContext, const sp<RenderController>& glResourceManager, const Viewport& viewport)
    : _graphics_context(new GraphicsContext(glContext, glResourceManager)), _viewport(viewport)
{
}

RenderViewOpenGL::~RenderViewOpenGL()
{
}

void RenderViewOpenGL::onSurfaceCreated()
{
    GLDebug::glPrintString("GL Version:", GL_VERSION);
    GLDebug::glPrintInteger("Max Uniform Components", GL_MAX_VERTEX_UNIFORM_COMPONENTS);
    GLDebug::glPrintInteger("Max Uniform Vectors", GL_MAX_VERTEX_UNIFORM_VECTORS);
}

void RenderViewOpenGL::onSurfaceChanged(uint32_t width, uint32_t height)
{
    _graphics_context.reset(new GraphicsContext(_graphics_context->renderContext(), _graphics_context->renderController()));
    initialize(width, height);
}

void RenderViewOpenGL::onRenderFrame(const Color& backgroundColor, const sp<RenderCommand>& renderCommand)
{
    glClearColor(backgroundColor.red(), backgroundColor.green(), backgroundColor.blue(), backgroundColor.alpha());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _graphics_context->onDrawFrame();
    renderCommand->draw(_graphics_context);
}

void RenderViewOpenGL::initialize(uint32_t width, uint32_t height)
{
    LOGD("Width: %d, Height: %d, Viewport (%.1f, %.1f, %.1f, %.1f)", width, height, _viewport.left(), _viewport.top(), _viewport.right(), _viewport.bottom());

    glViewport(0, 0, width, height);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);
#ifndef ANDROID
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

    _graphics_context->onSurfaceReady();
}

}
