#pragma once

#include <vector>

#include "core/base/bit_set.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/impl/renderer/render_group.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

namespace ark {

class RenderTarget final : public Renderer {
public:
    enum ClearMaskBits {
        CLEAR_MASK_NONE = 0,
        CLEAR_MASK_COLOR = 1,
        CLEAR_MASK_DEPTH = 2,
        CLEAR_MASK_STENCIL = 4,
        CLEAR_MASK_DEPTH_STENCIL = 6,
        CLEAR_MASK_ALL = 7
    };
    typedef BitSet<ClearMaskBits> ClearMask;

    struct CreateConfigure {
        std::vector<sp<Texture>> _color_attachments;
        sp<Texture> _depth_stencil_attachment;
        Texture::Flag _depth_stencil_flags;
        ClearMask _clear_mask;
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
        sp<Builder<RendererPhrase>> _renderer;
        std::vector<std::pair<sp<Builder<Texture>>, document>> _attachments;
        ClearMask _clear_mask;
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
