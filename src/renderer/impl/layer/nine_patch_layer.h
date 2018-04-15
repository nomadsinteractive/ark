#ifndef ARK_RENDERER_IMPL_LAYER_NINE_PATCH_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_NINE_PATCH_LAYER_H_

#include "core/inf/builder.h"

#include "graphics/inf/layer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "renderer/base/gl_elements.h"

namespace ark {

class NinePatchLayer : public Layer {
public:
    NinePatchLayer(const sp<Atlas>& atlas, const sp<GLShader>& shader, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RenderCommand> render(const LayerContext::Snapshot& layerContext, float x, float y) override;

//  [[plugin::resource-loader("nine-patch-layer")]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& parent, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Atlas>> _atlas;
        sp<Builder<GLShader>> _shader;
    };

private:
    GLElementsRenderer _nine_patch_elements;

};

}

#endif
