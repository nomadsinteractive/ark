#include "renderer/base/framebuffer.h"

#include "core/base/bean_factory.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

Framebuffer::Framebuffer(const sp<Resource>& resource, const sp<Renderer>& delegate)
    : _resource(resource), _delegate(delegate)
{
}

const sp<Resource>& Framebuffer::resource() const
{
    return _resource;
}

void Framebuffer::render(RenderRequest& renderRequest, const V3& position)
{
    _delegate->render(renderRequest, position);
}

Framebuffer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _renderer(factory.ensureBuilder<Renderer>(manifest, Constants::Attributes::DELEGATE)),
      _texture(factory.ensureBuilder<Texture>(manifest, Constants::Attributes::TEXTURE))
{
}

sp<Framebuffer> Framebuffer::BUILDER::build(const Scope& args)
{
    return _render_controller->makeFramebuffer(_renderer->build(args), _texture->build(args));
}

Framebuffer::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest)
    : _framebuffer(factory.ensureConcreteClassBuilder<Framebuffer>(manifest, "framebuffer"))
{
}

sp<Renderer> Framebuffer::RENDERER_BUILDER::build(const Scope& args)
{
    return _framebuffer->build(args);
}

}
