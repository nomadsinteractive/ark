#ifndef ARK_RENDERER_IMPL_LAYER_NINE_PATCH_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_NINE_PATCH_LAYER_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_layer.h"

#include "renderer/forwarding.h"

namespace ark {

class NinePatchLayer {
public:

//  [[plugin::resource-loader("nine-patch-layer")]]
    class BUILDER : public Builder<RenderLayer> {
    public:
        BUILDER(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<RenderLayer> build(const Scope& args) override;

    private:
        RenderLayer::BUILDER _impl;
    };

};

}

#endif
