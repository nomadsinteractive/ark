#pragma once

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/base/texture.h"

namespace ark::plugin::opengl {

class GLEquirectangularCubemapUploader final : public Texture::Uploader {
public:
    GLEquirectangularCubemapUploader(const Shader& shader, const sp<Texture>& texture, const sp<Size>& size);

    virtual void initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override;

//  [[plugin::resource-loader("equirectangular")]]
    class BUILDER : public Builder<Texture::Uploader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Texture::Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Size>> _size;
        sp<Builder<Shader>> _shader;
        sp<Builder<Texture>> _texture;
    };

private:
    sp<PipelineBindings> _pipeline_bindings;
    sp<Texture> _texture;
    sp<Size> _size;
};

}
