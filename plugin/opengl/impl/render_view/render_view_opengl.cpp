#include "opengl/impl/render_view/render_view_opengl.h"

#include "core/inf/array.h"
#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_backend_info.h"

#include "platform/gl/gl.h"

#include "opengl/util/gl_debug.h"
#include "renderer/base/render_backend.h"

namespace ark::plugin::opengl {

RenderViewOpenGL::RenderViewOpenGL(sp<RenderController> renderController)
    : _render_controller(std::move(renderController))
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

void RenderViewOpenGL::onSurfaceChanged(const uint32_t width, const uint32_t height)
{
    initialize(width, height);
}

void RenderViewOpenGL::onRenderFrame(const V4& backgroundColor, RenderCommand& renderCommand)
{
    glClearColor(backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), backgroundColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GraphicsContext graphicsContext(_render_controller);
    graphicsContext.onDrawFrame();
    renderCommand.draw(graphicsContext);
}

sp<Bitmap> RenderViewOpenGL::doScreenshot()
{
    constexpr uint8_t channels = 3;
    const auto [width, height] = _render_controller->renderBackend()->info()->displayResolution();
    const uint32_t rowBytes = width * channels;

    bytearray bytes = sp<ByteArray>::make<ByteArray::Allocated>(rowBytes * height);
    GL_CHECK_ERROR(glReadPixels(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height), GL_RGB, GL_UNSIGNED_BYTE, bytes->buf()));

    // OpenGL reads pixels bottom-to-top, flip vertically
    Bitmap::Util<uint8_t>::vflip(bytes->buf(), width, height, channels);

    return sp<Bitmap>::make(width, height, rowBytes, channels, std::move(bytes));
}

void RenderViewOpenGL::initialize(const uint32_t width, const uint32_t height)
{
    LOGD("Width: %d, Height: %d", width, height);
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

    _render_controller->onSurfaceReady(GraphicsContext::mocked());
}

}
