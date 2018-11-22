#include "renderer/impl/renderer/framebuffer_renderer.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

#include "graphics/base/render_request.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_framebuffer.h"
#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/resource_loader_context.h"

#include "renderer/inf/gl_resource.h"
#include "renderer/opengl/util/gl_debug.h"

namespace ark {

extern uint32_t g_GLViewportWidth;
extern uint32_t g_GLViewportHeight;

namespace {

class PreDrawElementsToFBO : public RenderCommand {
public:
    PreDrawElementsToFBO(const sp<GLFramebuffer>& fbo)
        : _fbo(fbo) {
    }

    virtual void draw(GraphicsContext& /*graphicsContext*/) override {
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo->id());
        glViewport(0, 0, static_cast<GLsizei>(_fbo->texture()->width()), static_cast<GLsizei>(_fbo->texture()->height()));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

private:
    sp<GLFramebuffer> _fbo;
};

class PostDrawElementsToFBO : public RenderCommand {
public:
    virtual void draw(GraphicsContext& /*graphicsContext*/) override {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, g_GLViewportWidth, g_GLViewportHeight);
    }

};

}

FrameBufferRenderer::FrameBufferRenderer(const sp<Renderer>& delegate, const sp<GLTexture>& texture, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _delegate(delegate), _fbo(sp<GLFramebuffer>::make(resourceLoaderContext->glResourceManager()->recycler(), texture)),
      _pre_draw(sp<PreDrawElementsToFBO>::make(_fbo)), _post_draw(sp<PostDrawElementsToFBO>::make())
{
    resourceLoaderContext->glResourceManager()->prepare(_fbo, GLResourceManager::PS_ONCE_AND_ON_SURFACE_READY);
}

void FrameBufferRenderer::render(RenderRequest& renderRequest, float x, float y)
{
    renderRequest.addRequest(_pre_draw);
    _delegate->render(renderRequest, x, y);
    renderRequest.addRequest(_post_draw);
}

FrameBufferRenderer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _delegate(factory.ensureBuilder<Renderer>(manifest, Constants::Attributes::DELEGATE)),
      _texture(factory.ensureBuilder<GLTexture>(manifest, "texture"))
{
}

sp<Renderer> FrameBufferRenderer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Renderer> delegate = _delegate->build(args);
    const sp<GLTexture> texture = _texture->build(args);
    return sp<FrameBufferRenderer>::make(delegate, texture, _resource_loader_context);
}

}
