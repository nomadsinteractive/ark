#include "renderer/base/framebuffer.h"

#include "core/base/bean_factory.h"
#include "core/util/conversions.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

Framebuffer::Framebuffer(const sp<Resource>& resource, const sp<Renderer>& renderer)
    : _delegate(resource), _renderer(renderer)
{
}

const sp<Resource>& Framebuffer::delegate() const
{
    return _delegate;
}

void Framebuffer::render(RenderRequest& renderRequest, const V3& position)
{
    _renderer->render(renderRequest, position);
}

Framebuffer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _renderer(factory.ensureBuilder<Renderer>(manifest, Constants::Attributes::DELEGATE)),
      _textures(factory.getBuilderList<Texture>(manifest, Constants::Attributes::TEXTURE)), _clear_mask(Documents::getAttribute<Framebuffer::ClearMask>(manifest, "clear-mask", Framebuffer::CLEAR_MASK_ALL))
{
    DCHECK(_textures.size(), "No texture defined in manifest: \"%s\"", Documents::toString(manifest).c_str());
}

sp<Framebuffer> Framebuffer::BUILDER::build(const Scope& args)
{
    std::vector<sp<Texture>> textures;
    for(const sp<Builder<Texture>>& i : _textures)
        textures.push_back(i->build(args));
    return _render_controller->makeFramebuffer(_renderer->build(args), std::move(textures), _clear_mask);
}

Framebuffer::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest)
    : _framebuffer(factory.ensureConcreteClassBuilder<Framebuffer>(manifest, "framebuffer"))
{
}

sp<Renderer> Framebuffer::RENDERER_BUILDER::build(const Scope& args)
{
    return _framebuffer->build(args);
}

template<> ARK_API Framebuffer::ClearMask Conversions::to<String, Framebuffer::ClearMask>(const String& str)
{
    if(str == "none")
        return Framebuffer::CLEAR_MASK_NONE;
    if(str == "all")
        return Framebuffer::CLEAR_MASK_ALL;

    int32_t clearMask = Framebuffer::CLEAR_MASK_NONE;
    for(const String& i : str.split('|'))
    {
        if(i == "color")
            clearMask |= Framebuffer::CLEAR_MASK_COLOR;
        else if(i == "depth")
            clearMask |= Framebuffer::CLEAR_MASK_DEPTH;
        else
        {
            DCHECK(i == "stencil", "Unknow ClearMask: \"%s\"", i.c_str());
            clearMask |= Framebuffer::CLEAR_MASK_STENCIL;
        }
    }
    return static_cast<Framebuffer::ClearMask>(clearMask);
}


}
