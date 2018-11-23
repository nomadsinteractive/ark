#ifndef ARK_RENDERER_IMPL_LAYER_NINE_PATCH_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_NINE_PATCH_LAYER_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class NinePatchLayer {
public:

//  [[plugin::resource-loader("nine-patch-layer")]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& parent, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Atlas>> _atlas;
        sp<Builder<GLPipeline>> _shader;
    };

};

}

#endif
