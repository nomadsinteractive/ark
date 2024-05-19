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

static void _glDebugCallbackProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    LOGD(message);
}

RenderViewOpenGL::RenderViewOpenGL(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
    : _graphics_context(new GraphicsContext(renderContext, renderController))
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
    GLDebug::glPrintInteger("Max Color Attachments", GL_MAX_COLOR_ATTACHMENTS);
    GLDebug::glPrintInteger("Max Draw Buffers", GL_MAX_DRAW_BUFFERS);

    int32_t work_grp_cnt[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
    LOGD("Max Compute Work Group Count: (%d, %d, %d)", work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

    int32_t work_grp_size[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
    LOGD("Max Compute Work Group Size: (%d, %d, %d)", work_grp_size[0], work_grp_size[1], work_grp_size[2]);
}

void RenderViewOpenGL::onSurfaceChanged(uint32_t width, uint32_t height)
{
    _graphics_context.reset(new GraphicsContext(_graphics_context->renderContext(), _graphics_context->renderController()));
    initialize(width, height);
}

void RenderViewOpenGL::onRenderFrame(const Color& backgroundColor, RenderCommand& renderCommand)
{
    const V4 rgba = backgroundColor.rgba();
    glClearColor(rgba.x(), rgba.y(), rgba.z(), rgba.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _graphics_context->onDrawFrame();
    renderCommand.draw(_graphics_context);
}

void RenderViewOpenGL::initialize(uint32_t width, uint32_t height)
{
    LOGD("Width: %d, Height: %d", width, height);
#if !defined(__APPLE__)
    glDebugMessageCallback(_glDebugCallbackProc, nullptr);
#endif
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

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
