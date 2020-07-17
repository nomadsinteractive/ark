#ifndef ARK_PLUGIN_CMFT_TEXTURE_UPLOADER_IRRADIANCE_CUBEMAP_UPLOADER_H_
#define ARK_PLUGIN_CMFT_TEXTURE_UPLOADER_IRRADIANCE_CUBEMAP_UPLOADER_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/base/texture.h"

namespace ark {
namespace plugin {
namespace cmft {

class IrradianceCubemapUploader : public Texture::Uploader {
public:
    IrradianceCubemapUploader(const sp<Texture>& texture, const sp<Size>& size);

    virtual void upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override;

//  [[plugin::builder("irradiance")]]
    class BUILDER : public Builder<Texture::Uploader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Texture::Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Size>> _size;
        sp<Builder<Texture>> _texture;
    };

private:
    sp<Texture> _texture;
    sp<Size> _size;
};

}
}
}

#endif
