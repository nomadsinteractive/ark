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
    PreDrawElementsToFBO(const sp<GLFramebuffer>& fbo, int32_t clearMask)
        : _fbo(fbo), _clear_mask(toClearBufferMask(clearMask)) {
    }

    virtual void draw(GraphicsContext& /*graphicsContext*/) override {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(_fbo->id()));
        const sp<Texture>& texture = _fbo->textures().at(0);
        glViewport(0, 0, static_cast<GLsizei>(texture->width()), static_cast<GLsizei>(texture->height()));
        if(_clear_mask != GL_NONE_BIT)
            glClear(_clear_mask);
    }

private:
    ClearBufferMask toClearBufferMask(int32_t clearMask) const {
        ClearBufferMask cm = GL_NONE_BIT;
        if(clearMask & Framebuffer::CLEAR_MASK_COLOR)
            cm |= GL_COLOR_BUFFER_BIT;
        if(clearMask & Framebuffer::CLEAR_MASK_DEPTH)
            cm |= GL_DEPTH_BUFFER_BIT;
        if(clearMask & Framebuffer::CLEAR_MASK_STENCIL)
            cm |= GL_STENCIL_BUFFER_BIT;
        return cm;
    }

private:
    sp<GLFramebuffer> _fbo;
    ClearBufferMask _clear_mask;
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

GLFramebufferRenderer::GLFramebufferRenderer(sp<Renderer> delegate, sp<GLFramebuffer> framebuffer, int32_t clearMask)
    : _delegate(std::move(delegate)), _fbo(std::move(framebuffer)), _pre_draw(sp<PreDrawElementsToFBO>::make(_fbo, clearMask)), _post_draw(sp<PostDrawElementsToFBO>::make())
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
