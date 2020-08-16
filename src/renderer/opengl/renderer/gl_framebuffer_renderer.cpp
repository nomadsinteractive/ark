#include "renderer/opengl/renderer/gl_framebuffer_renderer.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

#include "graphics/base/render_request.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/buffer.h"
#include "renderer/base/framebuffer.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/texture.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/resource.h"

#include "renderer/opengl/base/gl_framebuffer.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

namespace {

class PreDrawElementsToFBO : public RenderCommand {
public:
    PreDrawElementsToFBO(sp<Framebuffer> fbo, const std::vector<sp<Texture>>& drawBuffers, int32_t clearMask)
        : _fbo(std::move(fbo)), _width(_fbo->width()), _height(_fbo->height()), _clear_mask(clearMask),
          _clear_color_value{0, 0, 0, 0}, _clear_depth_value(1.0f), _clear_stencil_value(0) {
        const std::vector<sp<Texture>>& colorAttachments = _fbo->colorAttachments();
        for(const sp<Texture>& i : drawBuffers) {
            const auto iter = std::find(colorAttachments.begin(), colorAttachments.end(), i);
            DCHECK(iter != colorAttachments.end(), "Texture does not belong to Framebuffer's color attachments");
            _draw_buffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + static_cast<uint32_t>(iter - colorAttachments.begin())));
        }
    }

    virtual void draw(GraphicsContext& /*graphicsContext*/) override {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(_fbo->delegate()->id()));
        glViewport(0, 0, _width, _height);
        glDrawBuffers(static_cast<GLsizei>(_draw_buffers.size()), _draw_buffers.data());
        if(_clear_mask) {
            uint32_t mds = _clear_mask & Framebuffer::CLEAR_MASK_DEPTH_STENCIL;
            if(mds) {
                if(mds == Framebuffer::CLEAR_MASK_DEPTH_STENCIL)
                    glClearBufferfi(GL_DEPTH_STENCIL, 0, _clear_depth_value, _clear_stencil_value);
                else if(mds == Framebuffer::CLEAR_MASK_DEPTH)
                    glClearBufferfv(GL_DEPTH, 0, &_clear_depth_value);
                else
                    glClearBufferiv(GL_STENCIL, 0, &_clear_stencil_value);
            }

            if(_clear_mask & Framebuffer::CLEAR_MASK_COLOR)
                for(size_t i = 0; i < _draw_buffers.size(); ++i)
                    glClearBufferfv(GL_COLOR, static_cast<GLint>(i), _clear_color_value);
        }
    }

private:
    sp<Framebuffer> _fbo;
    GLsizei _width, _height;
    int32_t _clear_mask;

    float _clear_color_value[4];
    float _clear_depth_value;
    int32_t _clear_stencil_value;
    std::vector<GLenum> _draw_buffers;
};

class PostDrawElementsToFBO : public RenderCommand {
public:
    virtual void draw(GraphicsContext& graphicsContext) override {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        const RenderEngineContext::Resolution& resolution = graphicsContext.renderContext()->displayResolution();
        glViewport(0, 0, static_cast<GLsizei>(resolution.width), static_cast<GLsizei>(resolution.height));
    }

};

}

GLFramebufferRenderer::GLFramebufferRenderer(sp<Framebuffer> framebuffer, sp<Renderer> delegate, std::vector<sp<Texture>> drawBuffers, int32_t clearMask)
    : _delegate(std::move(delegate)), _pre_draw(sp<PreDrawElementsToFBO>::make(std::move(framebuffer), drawBuffers, clearMask)), _post_draw(sp<PostDrawElementsToFBO>::make())
{
}

void GLFramebufferRenderer::render(RenderRequest& renderRequest, const V3& position)
{
    renderRequest.addRequest(_pre_draw);
    _delegate->render(renderRequest, position);
    renderRequest.addRequest(_post_draw);
}

}
}
