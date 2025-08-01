#include "renderer/base/render_target.h"

#include <ranges>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/util/string_convert.h"

#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderTarget::RenderTarget(sp<Renderer> renderer, sp<Resource> resource)
    : _renderer(std::move(renderer)), _resource(std::move(resource))
{
}

void RenderTarget::render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator)
{
    _renderer->render(renderRequest, position, drawDecorator);
}

const sp<Resource>& RenderTarget::resource() const
{
    return _resource;
}

RenderTarget::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _renderer(factory.getBuilder<Renderer>(manifest, constants::RENDERER)), _render_layer(factory.getBuilder<RenderLayer>(manifest, constants::RENDER_LAYER)),
      _clear_mask(Documents::getAttribute<ClearBitSet>(manifest, "clear-mask", CLEAR_BIT_ALL)), _color_attachment_op(Documents::getAttribute<AttachmentOp>(manifest, "color-attachment-op", {ATTACHMENT_OP_BIT_LOAD, ATTACHMENT_OP_BIT_STORE})),
      _depth_stencil_op(Documents::getAttribute<AttachmentOp>(manifest, "depth-stencil-op", {ATTACHMENT_OP_BIT_LOAD, ATTACHMENT_OP_BIT_STORE}))
{
    for(const document& i : manifest->children(constants::TEXTURE))
        _attachments.emplace_back(factory.ensureBuilder<Texture>(i), i);

    CHECK(_attachments.size(), "No texture attachment defined in manifest: \"%s\"", Documents::toString(manifest).c_str());
}

sp<RenderTarget> RenderTarget::BUILDER::build(const Scope& args)
{
    Configure configure = {_color_attachment_op, _depth_stencil_op, _clear_mask};
    for(const auto& i : std::views::keys(_attachments))
    {
        sp<Texture> tex = i->build(args);
        if(const Texture::Usage usage = tex->usage(); usage == Texture::USAGE_AUTO || usage.has(Texture::USAGE_COLOR_ATTACHMENT))
            configure._color_attachments.push_back(std::move(tex));
        else
        {
            CHECK(configure._depth_stencil_attachment == nullptr, "Only one depth-stencil attachment allowed");
            CHECK(usage.has(Texture::USAGE_DEPTH_STENCIL_ATTACHMENT), "Unknow Texture usage: %d", usage);
            configure._depth_stencil_attachment = std::move(tex);
        }
    }

    sp<Renderer> renderer = _renderer.build(args);
    const sp<RenderLayer> renderLayer = _render_layer.build(args);
    ASSERT(renderer || renderLayer);
    CHECK(!(renderer && renderLayer), "Unimplemented");
    if(renderLayer)
        if(const auto& traits = renderLayer->_stub->_shader->pipelineDesciptor()->configuration()._traits; traits.has(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST))
            configure._depth_test_write_enabled = traits.at(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST)._depth_test._write_enabled;
    return Ark::instance().renderController()->makeRenderTarget(renderer ? std::move(renderer) : renderLayer.cast<Renderer>(), std::move(configure));
}

RenderTarget::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest)
    : _impl(factory, manifest)
{
}

sp<Renderer> RenderTarget::RENDERER_BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

template<> ARK_API RenderTarget::ClearBitSet StringConvert::eval<RenderTarget::ClearBitSet>(const String& str)
{
    constexpr RenderTarget::ClearBitSet::LookupTable<5> clearBits = {{
            {"none", RenderTarget::CLEAR_BIT_NONE},
            {"color", RenderTarget::CLEAR_BIT_COLOR},
            {"depth", RenderTarget::CLEAR_BIT_DEPTH},
            {"stencil", RenderTarget::CLEAR_BIT_STENCIL},
            {"all", RenderTarget::CLEAR_BIT_ALL}
        }};
    return RenderTarget::ClearBitSet::toBitSet(str, clearBits);
}

template<> RenderTarget::AttachmentOp StringConvert::eval<RenderTarget::AttachmentOp>(const String& str)
{
    constexpr RenderTarget::AttachmentOp::LookupTable<4> opBits = {{
        {"load", RenderTarget::ATTACHMENT_OP_BIT_LOAD},
        {"clear", RenderTarget::ATTACHMENT_OP_BIT_CLEAR},
        {"dont_care", RenderTarget::ATTACHMENT_OP_BIT_DONT_CARE},
        {"store", RenderTarget::ATTACHMENT_OP_BIT_STORE},
    }};
    if(str)
        return RenderTarget::AttachmentOp::toBitSet(str, opBits);
    return {RenderTarget::ATTACHMENT_OP_BIT_LOAD, RenderTarget::ATTACHMENT_OP_BIT_STORE};
}

}
