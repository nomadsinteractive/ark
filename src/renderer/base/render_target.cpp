#include "renderer/base/render_target.h"

#include "core/base/bean_factory.h"
#include "core/util/string_convert.h"

#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderTarget::RenderTarget(sp<Renderer> renderer, sp<Resource> resource)
    : _renderer(std::move(renderer)), _resource(std::move(resource))
{
}

void RenderTarget::render(RenderRequest& renderRequest, const V3& position)
{
    _renderer->render(renderRequest, position);
}

const sp<Resource>& RenderTarget::resource() const
{
    return _resource;
}

RenderTarget::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _renderer(factory.ensureBuilder<RendererPhrase>(manifest)),
      _clear_mask(Documents::getAttribute<RenderTarget::ClearMask>(manifest, "clear-mask", CLEAR_MASK_ALL))
{
    for(const document& i : manifest->children(constants::TEXTURE))
        _attachments.emplace_back(factory.ensureBuilder<Texture>(i), i);

    CHECK(_attachments.size(), "No texture attachment defined in manifest: \"%s\"", Documents::toString(manifest).c_str());
}

sp<RenderTarget> RenderTarget::BUILDER::build(const Scope& args)
{
    CreateConfigure configure;
    for(const auto& [i, j] : _attachments)
    {
        sp<Texture> tex = i->build(args);
        if(const Texture::Usage usage = tex->usage(); usage == Texture::USAGE_AUTO || usage.has(Texture::USAGE_COLOR_ATTACHMENT))
            configure._color_attachments.push_back(std::move(tex));
        else
        {
            CHECK(configure._depth_stencil_attachment == nullptr, "Only one depth-stencil attachment allowed");
            CHECK(usage.has(Texture::USAGE_DEPTH_STENCIL_ATTACHMENT), "Unknow Texture usage: %d", usage);
            const Texture::Flag flags = Documents::getAttribute<Texture::Flag>(j, "flags", Texture::FLAG_FOR_OUTPUT);
            CHECK_WARN(!(flags & Texture::FLAG_FOR_INPUT && usage.has(Texture::USAGE_DEPTH_STENCIL_ATTACHMENT) && _clear_mask.has(RenderTarget::CLEAR_MASK_DEPTH_STENCIL)),
                       "Depth-stencil texture marked \"for input\" would be cleared before rendering pass");
            configure._depth_stencil_flags = flags;
            configure._depth_stencil_attachment = std::move(tex);
        }
    }
    configure._clear_mask = _clear_mask;
    return _render_controller->makeRenderTarget(_renderer->build(args), std::move(configure));
}

RenderTarget::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext)
{
}

sp<Renderer> RenderTarget::RENDERER_BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

template<> ARK_API RenderTarget::ClearMask StringConvert::eval<RenderTarget::ClearMask>(const String& str)
{
    constexpr std::array<std::pair<const char*, RenderTarget::ClearMaskBits>, 5> clearMasks = {{
            {"none", RenderTarget::CLEAR_MASK_NONE},
            {"color", RenderTarget::CLEAR_MASK_COLOR},
            {"depth", RenderTarget::CLEAR_MASK_DEPTH},
            {"stencil", RenderTarget::CLEAR_MASK_COLOR},
            {"all", RenderTarget::CLEAR_MASK_ALL}
        }};
    return RenderTarget::ClearMask::toBitSet(str, clearMasks);
}

}