#include "opengl/impl/renderer/gl_framebuffer_renderer.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/render_target.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine_context.h"

#include "opengl/base/gl_framebuffer.h"

#include "platform/gl/gl.h"

#include "renderer/util/render_util.h"

namespace ark::plugin::opengl {

struct GLFramebufferRenderer::Stub {
    sp<GLFramebuffer> _framebuffer;
    Vector<uint32_t> _blending_disabled_attachments;
};

namespace {

class PreDrawElementsToFBO final : public RenderCommand {
public:
    PreDrawElementsToFBO(const sp<GLFramebufferRenderer::Stub>& stub, const int32_t width, const int32_t height)
        : _stub(stub), _width(width), _height(height), _clear_depth_value(1.0f), _clear_stencil_value(0) {
    }

    void draw(GraphicsContext& /*graphicsContext*/) override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(_stub->_framebuffer->id()));
        glViewport(0, 0, _width, _height);
        const RenderTarget::Configure& configure = _stub->_framebuffer->configure();
        if(configure._depth_attachment_op.contains(RenderTarget::ATTACHMENT_OP_BIT_CLEAR))
        {
            if(configure._depth_stencil_attachment->usage().has(Texture::USAGE_DEPTH_STENCIL_ATTACHMENT))
                glClearBufferfi(GL_DEPTH_STENCIL, 0, _clear_depth_value, _clear_stencil_value);
            else if(configure._depth_stencil_attachment->usage().has(Texture::USAGE_DEPTH_ATTACHMENT))
                glClearBufferfv(GL_DEPTH, 0, &_clear_depth_value);
            else
                glClearBufferiv(GL_STENCIL, 0, &_clear_stencil_value);
        }

        if(configure._color_attachment_op.contains(RenderTarget::ATTACHMENT_OP_BIT_CLEAR))
            for(size_t i = 0; i < configure._color_attachments.size(); ++i)
                glClearBufferfv(GL_COLOR, static_cast<GLint>(i), reinterpret_cast<const GLfloat *>(&configure._color_attachments.at(i)._clear_value));

        uint32_t attachmentIndex = 0;
        for(const auto& [t, _] : configure._color_attachments)
        {
            if(!RenderUtil::shouldSupportAlphaBlending(t->parameters()->_format) && glIsEnabledi(GL_BLEND, attachmentIndex))
            {
                _stub->_blending_disabled_attachments.push_back(attachmentIndex);
                glDisablei(GL_BLEND, attachmentIndex);
            }
            attachmentIndex++;
        }
    }

private:
    sp<GLFramebufferRenderer::Stub> _stub;
    GLsizei _width, _height;
    float _clear_depth_value;
    int32_t _clear_stencil_value;
};

class PostDrawElementsToFBO final : public RenderCommand {
public:
    PostDrawElementsToFBO(const sp<GLFramebufferRenderer::Stub>& stub)
        : _stub(stub) {
    }
    void draw(GraphicsContext& graphicsContext) override
    {
        for(const uint32_t i : _stub->_blending_disabled_attachments)
            glEnablei(GL_BLEND, i);
        _stub->_blending_disabled_attachments.clear();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        const auto [width, height] = graphicsContext.renderContext()->displayResolution();
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }

private:
    sp<GLFramebufferRenderer::Stub> _stub;
};

}

GLFramebufferRenderer::GLFramebufferRenderer(sp<GLFramebuffer> fbo, int32_t width, int32_t height, sp<Renderer> renderer)
    : _stub(sp<Stub>::make(Stub{std::move(fbo)})), _renderer(std::move(renderer)), _pre_draw(sp<RenderCommand>::make<PreDrawElementsToFBO>(_stub, width, height)), _post_draw(sp<RenderCommand>::make<PostDrawElementsToFBO>(_stub))
{
}

void GLFramebufferRenderer::render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator)
{
    renderRequest.addRenderCommand(_pre_draw);
    _renderer->render(renderRequest, position, drawDecorator);
    renderRequest.addRenderCommand(_post_draw);
}

}
