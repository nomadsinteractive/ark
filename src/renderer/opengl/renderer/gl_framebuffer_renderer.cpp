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
        : _fbo(std::move(fbo)), _width(_fbo->width()), _height(_fbo->height()), _clear_mask(toClearBufferMask(clearMask)) {
        const std::vector<sp<Texture>>& colorAttachments = _fbo->colorAttachments();
        for(const sp<Texture>& i : drawBuffers) {
            const auto iter = std::find(colorAttachments.begin(), colorAttachments.end(), i);
            DCHECK(iter != colorAttachments.end(), "Texture does not belong to Framebuffer's color attachment");
            _draw_buffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + (iter - colorAttachments.begin())));
        }
    }

    virtual void draw(GraphicsContext& /*graphicsContext*/) override {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(_fbo->delegate()->id()));
        glViewport(0, 0, _width, _height);
        glDrawBuffers(static_cast<GLsizei>(_draw_buffers.size()), _draw_buffers.data());
        if(_clear_mask != 0)
            glClear(static_cast<decltype(GL_COLOR_BUFFER_BIT)>(_clear_mask));
    }

private:
    uint32_t toClearBufferMask(int32_t clearMask) const {
        uint32_t cm = 0;
        if(clearMask & Framebuffer::CLEAR_MASK_COLOR)
            cm |= static_cast<uint32_t>(GL_COLOR_BUFFER_BIT);
        if(clearMask & Framebuffer::CLEAR_MASK_DEPTH)
            cm |= static_cast<uint32_t>(GL_DEPTH_BUFFER_BIT);
        if(clearMask & Framebuffer::CLEAR_MASK_STENCIL)
            cm |= static_cast<uint32_t>(GL_STENCIL_BUFFER_BIT);
        return cm;
    }

private:
    sp<Framebuffer> _fbo;
    GLsizei _width, _height;
    uint32_t _clear_mask;

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
