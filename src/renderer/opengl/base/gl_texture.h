#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

#include "renderer/opengl/forwarding.h"

namespace ark {
namespace opengl {

class GLTexture : public Texture::Delegate {
public:
    GLTexture(sp<Recycler> recycler, sp<Size> size, uint32_t target, Texture::Type type, sp<Texture::Parameters> parameters);
    virtual ~GLTexture() override;

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader) override;
    virtual ResourceRecycleFunc recycle() override;

    virtual void clear(GraphicsContext& /*graphicsContext*/) override;

    int32_t width() const;
    int32_t height() const;

    uint32_t target() const;

    const sp<GLRenderbuffer>& renderbuffer() const;
    void setRenderbuffer(sp<GLRenderbuffer> renderbuffer);

private:
    ResourceRecycleFunc doRecycle();

protected:
    sp<Recycler> _recycler;
    sp<Size> _size;
    uint32_t _target;
    sp<Texture::Parameters> _parameters;

    sp<GLRenderbuffer> _renderbuffer;

    uint32_t _id;
    uint32_t _fbo;
};

}
}
