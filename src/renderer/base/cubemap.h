#ifndef ARK_RENDERER_BASE_CUBEMAP_H_
#define ARK_RENDERER_BASE_CUBEMAP_H_

#include "renderer/base/texture.h"

namespace ark {

class Cubemap {
public:
//  [[plugin::resource-loader("cubemap")]]
    class BUILDER : public Builder<Texture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        BeanFactory _factory;
        document _manifest;
        SafePtr<Builder<String>> _src;
        sp<Texture::Parameters> _parameters;
    };

};

}

#endif
