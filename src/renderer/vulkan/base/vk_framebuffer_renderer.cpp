#include "renderer/vulkan/base/vk_framebuffer_renderer.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

#include "graphics/base/render_request.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/buffer.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_context.h"
#include "renderer/base/texture.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/resource.h"

#include "renderer/vulkan/base/vk_framebuffer.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

namespace {

class PreDrawElementsToFBO : public RenderCommand {
public:
    PreDrawElementsToFBO(const sp<VKFramebuffer>& fbo)
        : _fbo(fbo) {
    }

    virtual void draw(GraphicsContext& /*graphicsContext*/) override {
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo->id());
        glViewport(0, 0, static_cast<GLsizei>(_fbo->texture()->width()), static_cast<GLsizei>(_fbo->texture()->height()));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

private:
    sp<VKFramebuffer> _fbo;
};

class PostDrawElementsToFBO : public RenderCommand {
public:
    virtual void draw(GraphicsContext& graphicsContext) override {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        const V2& resolution = graphicsContext.renderContext()->resolution();
        glViewport(0, 0, static_cast<GLsizei>(resolution.x()), static_cast<GLsizei>(resolution.y()));
    }

};

}

VKFramebufferRenderer::VKFramebufferRenderer(const sp<Renderer>& delegate, const sp<VKFramebuffer>& framebuffer)
    : _delegate(delegate), _fbo(framebuffer), _pre_draw(sp<PreDrawElementsToFBO>::make(_fbo)), _post_draw(sp<PostDrawElementsToFBO>::make())
{
}

void VKFramebufferRenderer::render(RenderRequest& renderRequest, float x, float y)
{
    renderRequest.addRequest(_pre_draw);
    _delegate->render(renderRequest, x, y);
    renderRequest.addRequest(_post_draw);
}

}
}