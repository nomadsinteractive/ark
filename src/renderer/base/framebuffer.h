#ifndef ARK_RENDERER_BASE_FRAMEBUFFER_H_
#define ARK_RENDERER_BASE_FRAMEBUFFER_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

namespace ark {

class Framebuffer : public Renderer {
public:
    Framebuffer(const sp<Resource>& resource, const sp<Renderer>& delegate);

    const sp<Resource>& resource() const;

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Framebuffer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Framebuffer> build(const Scope& args) override;

    private:
        sp<RenderController> _render_controller;
        sp<Builder<Renderer>> _renderer;
        sp<Builder<Texture>> _texture;
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
    sp<Resource> _resource;
    sp<Renderer> _delegate;

};

}

#endif
