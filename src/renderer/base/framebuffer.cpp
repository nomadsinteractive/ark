#include "renderer/base/framebuffer.h"

#include "core/base/bean_factory.h"
#include "core/util/string_convert.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

Framebuffer::Framebuffer(sp<Renderer> renderer, sp<Resource> delegate)
    : _renderer(renderer), _delegate(std::move(delegate))
{
}

void Framebuffer::render(RenderRequest& renderRequest, const V3& position)
{
    _renderer->render(renderRequest, position);
}

const sp<Resource>& Framebuffer::delegate() const
{
    return _delegate;
}

Framebuffer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _renderer(factory.ensureBuilder<Renderer>(manifest, constants::DELEGATE)),
      _clear_mask(Documents::getAttribute<Framebuffer::ClearMask>(manifest, "clear-mask", Framebuffer::CLEAR_MASK_ALL))
{
    for(const document& i : manifest->children(constants::TEXTURE))
        _textures.emplace_back(factory.ensureBuilder<Texture>(i), i);

    CHECK(_textures.size(), "No texture attachment defined in manifest: \"%s\"", Documents::toString(manifest).c_str());
}

sp<Framebuffer> Framebuffer::BUILDER::build(const Scope& args)
{
    std::vector<sp<Texture>> colorAttachments;
    sp<Texture> depthStencilAttachments;
    for(const auto& [i, j] : _textures)
    {
        sp<Texture> tex = i->build(args);
        if(tex->usage() == Texture::USAGE_COLOR_ATTACHMENT)
            colorAttachments.push_back(std::move(tex));
        else
        {
            CHECK(depthStencilAttachments == nullptr, "Only one depth-stencil attachment allowed");
            CHECK(tex->usage() & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT, "Unknow Texture usage: %d", tex->usage());
            Texture::Flag flags = Documents::getAttribute<Texture::Flag>(j, "flags", Texture::FLAG_NONE);
            if(flags != Texture::FLAG_NONE)
            {
                sp<Texture::Parameters> tp = sp<Texture::Parameters>::make(*tex->parameters());
                sp<Texture> newTexture = sp<Texture>::make(*tex);
                tp->_flags = flags;
                CHECK_WARN(!(flags == Texture::FLAG_FOR_INPUT && (tex->usage() & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT) && _clear_mask & Framebuffer::CLEAR_MASK_DEPTH_STENCIL),
                           "Depth-stencil texture marked \"for input\" would be cleared before rendering pass");
                newTexture->setParameters(std::move(tp));
                tex = std::move(newTexture);
            }
            depthStencilAttachments = std::move(tex);
        }
    }
    return _render_controller->makeFramebuffer(_renderer->build(args), std::move(colorAttachments), std::move(depthStencilAttachments), _clear_mask);
}

Framebuffer::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _framebuffer(factory.ensureConcreteClassBuilder<Framebuffer>(manifest, "framebuffer"))
{
}

sp<Renderer> Framebuffer::RENDERER_BUILDER::build(const Scope& args)
{
    return _framebuffer->build(args);
}

template<> ARK_API Framebuffer::ClearMask StringConvert::eval<Framebuffer::ClearMask>(const String& str)
{
    return EnumMap<Framebuffer::ClearMask>::instance().toEnumCombo(str);
}

template<> void EnumMap<Framebuffer::ClearMask>::initialize(std::map<String, Framebuffer::ClearMask>& enums)
{
    enums["none"] = Framebuffer::CLEAR_MASK_NONE;
    enums["all"] = Framebuffer::CLEAR_MASK_ALL;
    enums["color"] = Framebuffer::CLEAR_MASK_COLOR;
    enums["depth"] = Framebuffer::CLEAR_MASK_DEPTH;
    enums["stencil"] = Framebuffer::CLEAR_MASK_STENCIL;
}

}
