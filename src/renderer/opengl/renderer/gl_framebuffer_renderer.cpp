#include "renderer/opengl/renderer/gl_framebuffer_renderer.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

#include "graphics/base/render_request.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/buffer.h"
#include "renderer/base/recycler.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/texture.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/resource.h"

#include "renderer/opengl/base/gl_framebuffer.h"

#include "platform/gl/gl.h"

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

GLFramebufferRenderer::GLFramebufferRenderer(const sp<Renderer>& delegate, const sp<Texture>& texture, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _delegate(delegate), _fbo(sp<GLFramebuffer>::make(resourceLoaderContext->resourceManager()->recycler(), texture)),
      _pre_draw(sp<PreDrawElementsToFBO>::make(_fbo)), _post_draw(sp<PostDrawElementsToFBO>::make())
{
    resourceLoaderContext->resourceManager()->upload(_fbo, ResourceManager::US_ONCE_AND_ON_SURFACE_READY);
}

void GLFramebufferRenderer::render(RenderRequest& renderRequest, float x, float y)
{
    renderRequest.addRequest(_pre_draw);
    _delegate->render(renderRequest, x, y);
    renderRequest.addRequest(_post_draw);
}

GLFramebufferRenderer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _delegate(factory.ensureBuilder<Renderer>(manifest, Constants::Attributes::DELEGATE)),
      _texture(factory.ensureBuilder<Texture>(manifest, "texture"))
{
}

sp<Renderer> GLFramebufferRenderer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Renderer> delegate = _delegate->build(args);
    const sp<Texture> texture = _texture->build(args);
    return sp<GLFramebufferRenderer>::make(delegate, texture, _resource_loader_context);
}

}
