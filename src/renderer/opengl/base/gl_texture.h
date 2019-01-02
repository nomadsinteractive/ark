#ifndef ARK_RENDERER_OPENGL_BASE_TEXTURE_H_
#define ARK_RENDERER_OPENGL_BASE_TEXTURE_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API GLTexture : public Resource {
public:
    GLTexture(const sp<Recycler>& recycler, const sp<Size>& size, uint32_t target, const sp<Texture::Parameters>& parameters);
    virtual ~GLTexture() override;

    virtual uintptr_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual RecycleFunc recycle() override;

    int32_t width() const;
    int32_t height() const;

    uint32_t target() const;

protected:
    virtual void doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id) = 0;

    sp<Recycler> _recycler;
    sp<Size> _size;
    uint32_t _target;
    sp<Texture::Parameters> _parameters;

    uint32_t _id;
};

}

#endif
