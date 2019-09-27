#ifndef ARK_RENDERER_IMPL_LAYER_TEXT_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_TEXT_LAYER_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_layer.h"

#include "renderer/forwarding.h"

namespace ark {

class TextLayer {
public:

//  [[plugin::resource-loader("text-layer")]]
    class BUILDER : public Builder<RenderLayer> {
    public:
        BUILDER(BeanFactory& parent, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<RenderLayer> build(const Scope& args);

    private:
        RenderLayer::BUILDER _impl;
    };
};

}

#endif
