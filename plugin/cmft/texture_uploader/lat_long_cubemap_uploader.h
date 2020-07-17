#ifndef ARK_PLUGIN_CMFT_TEXTURE_UPLOADER_LAT_LONG_CUBEMAP_UPLOADER_H_
#define ARK_PLUGIN_CMFT_TEXTURE_UPLOADER_LAT_LONG_CUBEMAP_UPLOADER_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/base/texture.h"

namespace ark {
namespace plugin {
namespace cmft {

class LatLongCubemapUploader : public Texture::Uploader {
public:
    LatLongCubemapUploader(sp<Bitmap> bitmap, sp<Size> size);

    virtual void upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override;

//  [[plugin::builder("lat-long")]]
    class BUILDER : public Builder<Texture::Uploader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Texture::Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Bitmap>> _bitmap;
        sp<Builder<Size>> _size;
    };

private:
    sp<Bitmap> _bitmap;
    sp<Size> _size;
};

}
}
}

#endif
