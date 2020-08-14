#ifndef ARK_RENDERER_BASE_FRAMEBUFFER_H_
#define ARK_RENDERER_BASE_FRAMEBUFFER_H_

#include <vector>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

namespace ark {

class Framebuffer {
public:
    enum ClearMask {
        CLEAR_MASK_NONE = 0,
        CLEAR_MASK_COLOR = 1,
        CLEAR_MASK_DEPTH = 2,
        CLEAR_MASK_STENCIL = 4,
        CLEAR_MASK_DEPTH_STENCIL = 6,
        CLEAR_MASK_ALL = 7
    };

    Framebuffer(sp<Resource> delegate, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachment);

    const sp<Resource>& delegate() const;
    const std::vector<sp<Texture>>& colorAttachments() const;
    const sp<Texture>& depthStencilAttachment() const;

    int32_t width() const;
    int32_t height() const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Framebuffer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Framebuffer> build(const Scope& args) override;

    private:
        sp<RenderController> _render_controller;
        std::vector<sp<Builder<Texture>>> _textures;
    };

//  [[plugin::resource-loader("offscreen")]]
    class RENDERER_BUILDER : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<RenderController> _render_controller;
        sp<Builder<Framebuffer>> _framebuffer;
        sp<Builder<Renderer>> _delegate;
        std::vector<sp<Builder<Texture>>> _textures;
        ClearMask _clear_mask;
    };

private:
    sp<Resource> _delegate;

    std::vector<sp<Texture>> _color_attachments;
    sp<Texture> _depth_stencil_attachment;

    int32_t _width, _height;
};

}

#endif
