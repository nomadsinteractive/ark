#ifndef ARK_RENDERER_BASE_GL_FRAMEBUFFER_H_
#define ARK_RENDERER_BASE_GL_FRAMEBUFFER_H_

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_resource.h"

namespace ark {

class GLFramebuffer : public RenderResource {
private:
    class Recycler : public RenderResource {
    public:
        Recycler(uint32_t id);

        virtual uint32_t id() override;
        virtual void prepare(GraphicsContext&) override;
        virtual void recycle(GraphicsContext&) override;

    private:
        uint32_t _id;
    };

public:
    GLFramebuffer(const sp<GLRecycler>& recycler, const sp<Texture>& texture);
    ~GLFramebuffer();

    virtual uint32_t id() override;

    virtual void prepare(GraphicsContext& graphicsContext) override;
    virtual void recycle(GraphicsContext& /*graphicsContext*/) override;

    const sp<Texture>& texture() const;

private:
    sp<GLRecycler> _recycler;
    sp<Texture> _texture;

    uint32_t _id;
};

}

#endif
