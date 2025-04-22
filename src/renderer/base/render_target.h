#pragma once

#include "core/base/api.h"
#include "core/base/bit_set.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

namespace ark {

class ARK_API RenderTarget final : public Renderer {
public:
    enum ClearBits {
        CLEAR_BIT_NONE = 0,
        CLEAR_BIT_COLOR = 1,
        CLEAR_BIT_DEPTH = 2,
        CLEAR_BIT_STENCIL = 4,
        CLEAR_BIT_DEPTH_STENCIL = 6,
        CLEAR_BIT_ALL = 7
    };
    typedef BitSet<ClearBits> ClearBitSet;

    enum AttachmentOpBits {
        ATTACHMENT_OP_BIT_LOAD = 1,
        ATTACHMENT_OP_BIT_CLEAR = 2,
        ATTACHMENT_OP_BIT_DONT_CARE = 3,
        ATTACHMENT_OP_BIT_STORE = 4
    };
    typedef BitSet<AttachmentOpBits> AttachmentOp;

    struct Configure {
        AttachmentOp _color_attachment_op;
        AttachmentOp _depth_stencil_op;
        ClearBitSet _clear_bits;
        Vector<sp<Texture>> _color_attachments;
        sp<Texture> _depth_stencil_attachment;
        bool _depth_test_write_enabled = true;
    };

    RenderTarget(sp<Renderer> renderer, sp<Resource> resource);

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;

    const sp<Resource>& resource() const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<RenderTarget> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<RenderTarget> build(const Scope& args) override;

    private:
        SafeBuilder<Renderer> _renderer;
        SafeBuilder<RenderLayer> _render_layer;
        Vector<std::pair<sp<Builder<Texture>>, document>> _attachments;
        ClearBitSet _clear_mask;
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
    sp<Resource> _resource;
};

}
