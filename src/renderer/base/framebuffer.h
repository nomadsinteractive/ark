#ifndef ARK_RENDERER_BASE_FRAMEBUFFER_H_
#define ARK_RENDERER_BASE_FRAMEBUFFER_H_

#include <vector>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

namespace ark {

class Framebuffer : public Renderer {
public:
    enum ClearMask {
        CLEAR_MASK_NONE = 0,
        CLEAR_MASK_COLOR = 1,
        CLEAR_MASK_DEPTH = 2,
        CLEAR_MASK_STENCIL = 4,
        CLEAR_MASK_ALL = 7
    };

    Framebuffer(const sp<Resource>& delegate, const sp<Renderer>& renderer);

    const sp<Resource>& delegate() const;

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Framebuffer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Framebuffer> build(const Scope& args) override;

    private:
        sp<RenderController> _render_controller;
        sp<Builder<Renderer>> _renderer;
        std::vector<sp<Builder<Texture>>> _textures;
        ClearMask _clear_mask;
    };

//  [[plugin::builder("framebuffer")]]
    class RENDERER_BUILDER : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Framebuffer>> _framebuffer;
    };

private:
    sp<Resource> _delegate;
    sp<Renderer> _renderer;

};

}

#endif
