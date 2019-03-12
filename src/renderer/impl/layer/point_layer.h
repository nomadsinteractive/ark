#ifndef ARK_RENDERER_IMPL_LAYER_POINT_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_POINT_LAYER_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class PointLayer {
public:

//  [[plugin::resource-loader("point-layer")]]
    class BUILDER : public Builder<RenderLayer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<RenderLayer> build(const sp<Scope>& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Atlas>> _atlas;
        sp<Builder<Shader>> _shader;
    };

};

}

#endif
