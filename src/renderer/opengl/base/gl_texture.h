#ifndef ARK_RENDERER_OPENGL_BASE_TEXTURE_H_
#define ARK_RENDERER_OPENGL_BASE_TEXTURE_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

#include "renderer/opengl/base/gl_resource.h"
#include "renderer/inf/render_resource.h"

namespace ark {

class ARK_API GLTexture : public GLResource {
public:
    GLTexture(const sp<GLRecycler>& recycler, const sp<Size>& size, uint32_t target, const sp<Texture::Parameters>& parameters);
    virtual ~GLTexture() override;

    virtual uint32_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual Recycler recycle() override;

    int32_t width() const;
    int32_t height() const;

    uint32_t target() const;

protected:
    virtual void doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id) = 0;

    sp<GLRecycler> _recycler;
    sp<Size> _size;
    uint32_t _target;
    sp<Texture::Parameters> _parameters;

    uint32_t _id;
};

}

#endif
