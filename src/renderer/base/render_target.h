#pragma once

#include "core/base/api.h"
#include "core/base/bit_set.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API RenderTarget final : public Renderer {
public:
    enum AttachmentOpBits {
        ATTACHMENT_OP_BIT_LOAD = 1,
        ATTACHMENT_OP_BIT_CLEAR = 2,
        ATTACHMENT_OP_BIT_DONT_CARE = 3,
        ATTACHMENT_OP_BIT_STORE = 4
    };
    typedef BitSet<AttachmentOpBits> AttachmentOp;

    struct Attachment {
        sp<Texture> _texture;
        V4 _clear_value;
    };

    struct Configure {
        AttachmentOp _color_attachment_op;
        AttachmentOp _depth_attachment_op;
        Vector<Attachment> _color_attachments;
        sp<Texture> _depth_stencil_attachment;
        bool _depth_test_write_enabled = true;
    };

    RenderTarget(sp<Renderer> renderer, sp<Resource> fbo, sp<Renderer> fboRenderer);

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;

//  [[script::bindings::property]]
    const sp<Renderer>& renderer() const;
    const sp<Resource>& fbo() const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<RenderTarget> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<RenderTarget> build(const Scope& args) override;

    private:
        struct AttachmentBuilder {
            AttachmentBuilder(BeanFactory& factory, const document& manifest);

            sp<Builder<Texture>> _texture;
            V4 _clear_value;
        };

        sp<Builder<Renderer>> _renderer;
        Vector<AttachmentBuilder> _attachments;
        AttachmentOp _color_attachment_op;
        AttachmentOp _depth_stencil_op;
    };

//  [[plugin::builder("render-target")]]
    class RENDERER_BUILDER final : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest);

        sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

private:
    sp<Renderer> _renderer;
    sp<Resource> _fbo;
    sp<Renderer> _fbo_renderer;
};

}
