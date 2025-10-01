#include "renderer/base/render_target.h"

#include <ranges>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/util/string_convert.h"
#include "core/util/documents.h"

#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

RenderTarget::RenderTarget(sp<Renderer> renderer, sp<Resource> fbo, sp<Renderer> fboRenderer)
    : _renderer(std::move(renderer)), _fbo(std::move(fbo)), _fbo_renderer(std::move(fboRenderer))
{
}

void RenderTarget::render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator)
{
    _fbo_renderer->render(renderRequest, position, drawDecorator);
}

const sp<Renderer>& RenderTarget::renderer() const
{
    return _renderer;
}

const sp<Resource>& RenderTarget::fbo() const
{
    return _fbo;
}

RenderTarget::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _renderer(factory.ensureBuilder<Renderer>(manifest, constants::RENDERER)), _attachments(factory.makeBuilderListObject<AttachmentBuilder>(manifest, constants::TEXTURE)),
      _color_attachment_op(Documents::getAttribute<AttachmentOp>(manifest, "color-attachment-op", {ATTACHMENT_OP_BIT_LOAD, ATTACHMENT_OP_BIT_STORE})),
      _depth_stencil_op(Documents::getAttribute<AttachmentOp>(manifest, "depth-stencil-op", {ATTACHMENT_OP_BIT_LOAD, ATTACHMENT_OP_BIT_STORE}))
{
    CHECK(_attachments.size(), "No texture attachment defined in manifest: \"%s\"", Documents::toString(manifest).c_str());
}

sp<RenderTarget> RenderTarget::BUILDER::build(const Scope& args)
{
    Configure configure = {_color_attachment_op, _depth_stencil_op};
    for(const AttachmentBuilder& i : _attachments)
    {
        sp<Texture> tex = i._texture->build(args);
        if(const Texture::Usage usage = tex->usage(); usage == Texture::USAGE_AUTO || usage.has(Texture::USAGE_COLOR_ATTACHMENT))
            configure._color_attachments.emplace_back(std::move(tex), i._clear_value);
        else
        {
            CHECK(configure._depth_stencil_attachment == nullptr, "Only one depth-stencil attachment allowed");
            CHECK(usage.has(Texture::USAGE_DEPTH_STENCIL_ATTACHMENT), "Texture has no depth stencil usage: %d", usage);
            configure._depth_stencil_attachment = std::move(tex);
        }
    }
    return Ark::instance().renderController()->makeRenderTarget(_renderer->build(args), std::move(configure));
}

RenderTarget::BUILDER::AttachmentBuilder::AttachmentBuilder(BeanFactory& factory, const document& manifest)
    : _texture(factory.ensureBuilder<Texture>(manifest)), _clear_value(Documents::getAttributeValue<V4>(manifest, "clear-value", V4(0)))
{
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
        {"store", RenderTarget::ATTACHMENT_OP_BIT_STORE}
    }};
    if(str)
        return RenderTarget::AttachmentOp::toBitSet(str, opBits);
    return {RenderTarget::ATTACHMENT_OP_BIT_LOAD, RenderTarget::ATTACHMENT_OP_BIT_STORE};
}

}
