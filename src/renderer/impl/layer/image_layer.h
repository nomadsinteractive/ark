#ifndef ARK_RENDERER_IMPL_LAYER_IMAGE_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_IMAGE_LAYER_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ImageLayer {
public:

//  [[plugin::resource-loader("image-layer")]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Atlas>> _atlas;
        sp<Builder<GLPipeline>> _shader;
    };

};

}

#endif
