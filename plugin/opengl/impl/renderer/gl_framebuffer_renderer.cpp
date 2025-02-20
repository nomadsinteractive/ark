#include "opengl/impl/renderer/gl_framebuffer_renderer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/render_request.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/render_target.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine_context.h"

#include "opengl/base/gl_framebuffer.h"

#include "platform/gl/gl.h"

namespace ark::plugin::opengl {

namespace {

struct PreDrawElementsToFBO final : RenderCommand {
    PreDrawElementsToFBO(sp<GLFramebuffer> fbo, int32_t width, int32_t height, uint32_t drawBufferCount, int32_t clearMask)
        : _fbo(std::move(fbo)), _width(width), _height(height), _clear_mask(clearMask), _clear_color_value(0, 0, 0, 1), _clear_depth_value(1.0f),
          _clear_stencil_value(0), _draw_buffer_count(drawBufferCount) {
    }

    void draw(GraphicsContext& /*graphicsContext*/) override {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(_fbo->id()));
        glViewport(0, 0, _width, _height);
        if(_clear_mask) {
            if(const uint32_t mds = _clear_mask & RenderTarget::CLEAR_BIT_DEPTH_STENCIL) {
                if(mds == RenderTarget::CLEAR_BIT_DEPTH_STENCIL)
                    glClearBufferfi(GL_DEPTH_STENCIL, 0, _clear_depth_value, _clear_stencil_value);
                else if(mds == RenderTarget::CLEAR_BIT_DEPTH)
                    glClearBufferfv(GL_DEPTH, 0, &_clear_depth_value);
                else
                    glClearBufferiv(GL_STENCIL, 0, &_clear_stencil_value);
            }

            if(_clear_mask & RenderTarget::CLEAR_BIT_COLOR)
                for(size_t i = 0; i < _draw_buffer_count; ++i)
                    glClearBufferfv(GL_COLOR, static_cast<GLint>(i), reinterpret_cast<GLfloat *>(&_clear_color_value));
        }
    }

    sp<GLFramebuffer> _fbo;
    GLsizei _width, _height;
    int32_t _clear_mask;

    V4 _clear_color_value;
    float _clear_depth_value;
    int32_t _clear_stencil_value;

    uint32_t _draw_buffer_count;
};

struct PostDrawElementsToFBO final : RenderCommand {
    void draw(GraphicsContext& graphicsContext) override {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        const auto [width, height] = graphicsContext.renderContext()->displayResolution();
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }
};

}

GLFramebufferRenderer::GLFramebufferRenderer(sp<GLFramebuffer> fbo, int32_t width, int32_t height, sp<Renderer> delegate, uint32_t drawBufferCount, int32_t clearMask)
    : _delegate(std::move(delegate)), _pre_draw(sp<PreDrawElementsToFBO>::make(std::move(fbo), width, height, drawBufferCount, clearMask)), _post_draw(sp<PostDrawElementsToFBO>::make())
{
}

void GLFramebufferRenderer::render(RenderRequest& renderRequest, const V3& position)
{
    renderRequest.addRenderCommand(_pre_draw);
    _delegate->render(renderRequest, position);
    renderRequest.addRenderCommand(_post_draw);
}

}
