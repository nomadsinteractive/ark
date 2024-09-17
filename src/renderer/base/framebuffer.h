#pragma once

#include <vector>

#include "core/base/bit_set.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

namespace ark {

class Framebuffer final : public Renderer {
public:
    enum ClearMask {
        CLEAR_MASK_NONE = 0,
        CLEAR_MASK_COLOR = 1,
        CLEAR_MASK_DEPTH = 2,
        CLEAR_MASK_STENCIL = 4,
        CLEAR_MASK_DEPTH_STENCIL = 6,
        CLEAR_MASK_ALL = 7
    };

    typedef BitSet<ClearMask> ClearMaskBitSet;

    Framebuffer(sp<Renderer> renderer, sp<Resource> delegate);

    void render(RenderRequest& renderRequest, const V3& position) override;

    const sp<Resource>& resource() const;

//  [[plugin::resource-loader]]
    class BUILDER final : public Builder<Framebuffer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Framebuffer> build(const Scope& args) override;

    private:
        sp<RenderController> _render_controller;
        sp<Builder<Renderer>> _renderer;
        std::vector<std::pair<sp<Builder<Texture>>, document>> _textures;
        ClearMaskBitSet _clear_mask;
    };

//  [[plugin::resource-loader("offscreen")]]
    class RENDERER_BUILDER final : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Renderer> build(const Scope& args) override;

    private:
        sp<RenderController> _render_controller;
        sp<Builder<Framebuffer>> _framebuffer;
    };

private:
    sp<Renderer> _renderer;
    sp<Resource> _resource;
};

}
