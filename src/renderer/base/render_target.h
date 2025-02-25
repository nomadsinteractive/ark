#pragma once

#include "core/base/api.h"
#include "core/base/bit_set.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/impl/renderer/render_group.h"
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

    enum DepthStencilOpBits {
        DEPTH_STENCIL_OP_BIT_LOAD = 1,
        DEPTH_STENCIL_OP_BIT_CLEAR = 2,
        DEPTH_STENCIL_OP_BIT_DONT_CARE = 3,
        DEPTH_STENCIL_OP_BIT_STORE = 4
    };
    typedef BitSet<DepthStencilOpBits> DepthStencilOp;

    struct CreateConfigure {
        Vector<sp<Texture>> _color_attachments;
        sp<Texture> _depth_stencil_attachment;
        ClearBitSet _clear_bits;
        DepthStencilOp _depth_stencil_op;
    };

    RenderTarget(sp<Renderer> renderer, sp<Resource> resource);

    void render(RenderRequest& renderRequest, const V3& position) override;

    const sp<Resource>& resource() const;

//  [[plugin::resource-loader]]
    class BUILDER final : public Builder<RenderTarget> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<RenderTarget> build(const Scope& args) override;

    private:
        sp<RenderController> _render_controller;
        sp<Builder<RenderGroup>> _renderer;
        Vector<std::pair<sp<Builder<Texture>>, document>> _attachments;
        ClearBitSet _clear_mask;
        DepthStencilOp _depth_stencil_op;
    };

//  [[plugin::resource-loader("render-target")]]
    class RENDERER_BUILDER final : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

private:
    sp<Renderer> _renderer;
    sp<Resource> _resource;
};

}
