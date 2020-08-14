#include "renderer/base/framebuffer.h"

#include "core/base/bean_factory.h"
#include "core/util/conversions.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

Framebuffer::Framebuffer(sp<Resource> delegate, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachment)
    : _delegate(std::move(delegate)), _color_attachments(std::move(colorAttachments)), _depth_stencil_attachment(std::move(depthStencilAttachment))
{
    DCHECK(_color_attachments.size() > 0, "Framebuffer object should have at least one color attachment");
    _width = _color_attachments.at(0)->width();
    _height = _color_attachments.at(0)->height();
}

const sp<Resource>& Framebuffer::delegate() const
{
    return _delegate;
}

const std::vector<sp<Texture>>& Framebuffer::colorAttachments() const
{
    return _color_attachments;
}

const sp<Texture>& Framebuffer::depthStencilAttachment() const
{
    return _depth_stencil_attachment;
}

int32_t Framebuffer::width() const
{
    return _width;
}

int32_t Framebuffer::height() const
{
    return _height;
}

Framebuffer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _textures(factory.getBuilderList<Texture>(manifest, Constants::Attributes::TEXTURE))
{
    DCHECK(_textures.size(), "No texture attachment defined in manifest: \"%s\"", Documents::toString(manifest).c_str());
}

sp<Framebuffer> Framebuffer::BUILDER::build(const Scope& args)
{
    std::vector<sp<Texture>> colorAttachments;
    sp<Texture> depthStencilAttachments;
    for(const sp<Builder<Texture>>& i : _textures)
    {
        sp<Texture> tex = i->build(args);
        if(tex->usage() == Texture::USAGE_COLOR_ATTACHMENT)
            colorAttachments.push_back(std::move(tex));
        else
        {
            DCHECK(depthStencilAttachments, "Only one depth-stencil attachment is allowed");
            DCHECK(tex->usage() & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT, "Unknow Texture usage: %d", tex->usage());
            depthStencilAttachments = std::move(tex);
        }
    }
    return _render_controller->makeFramebuffer(std::move(colorAttachments), std::move(depthStencilAttachments));
}

Framebuffer::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _framebuffer(factory.ensureConcreteClassBuilder<Framebuffer>(manifest, "framebuffer")),
      _delegate(factory.ensureBuilder<Renderer>(manifest, Constants::Attributes::DELEGATE)), _textures(factory.getBuilderList<Texture>(manifest, Constants::Attributes::TEXTURE)),
      _clear_mask(Documents::getAttribute<Framebuffer::ClearMask>(manifest, "clear-mask", Framebuffer::CLEAR_MASK_ALL))
{
}

sp<Renderer> Framebuffer::RENDERER_BUILDER::build(const Scope& args)
{
    std::vector<sp<Texture>> drawBuffers;
    sp<Framebuffer> fbo = _framebuffer->build(args);
    for(const sp<Builder<Texture>>& i : _textures)
        drawBuffers.push_back(i->build(args));
    return _render_controller->renderEngine()->rendererFactory()->createFramebufferRenderer(std::move(fbo), _delegate->build(args), std::move(drawBuffers), _clear_mask);
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
