#ifndef ARK_RENDERER_OPENGL_BASE_TEXTURE_H_
#define ARK_RENDERER_OPENGL_BASE_TEXTURE_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

namespace ark {

class ARK_API GLTexture : public Texture::Delegate {
public:
    GLTexture(const sp<Recycler>& recycler, const sp<Size>& size, uint32_t target, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader);
    virtual ~GLTexture() override;

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

    int32_t width() const;
    int32_t height() const;

    uint32_t target() const;

protected:
    sp<Recycler> _recycler;
    sp<Size> _size;
    uint32_t _target;
    sp<Texture::Parameters> _parameters;
    sp<Texture::Uploader> _uploader;

    uint32_t _id;
};

}

#endif
