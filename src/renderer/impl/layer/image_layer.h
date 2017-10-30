#ifndef ARK_RENDERER_IMPL_LAYER_IMAGE_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_IMAGE_LAYER_H_

#include "graphics/base/layer_context.h"
#include "graphics/inf/layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_elements.h"

namespace ark {

class ImageLayer : public Layer {
public:
    ImageLayer(const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(const LayerContext& renderContext, RenderCommandPipeline& pipeline, float x, float y) override;

    const sp<Atlas>& atlas() const;

//  [[plugin::resource-loader("image-layer")]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Atlas>> _atlas;
        sp<Builder<GLShader>> _shader;
    };

private:
    GLElements _elements;
    sp<Atlas> _atlas;
};

}

#endif
