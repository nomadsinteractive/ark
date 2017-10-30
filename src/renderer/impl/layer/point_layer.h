#ifndef ARK_RENDERER_IMPL_LAYER_POINT_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_POINT_LAYER_H_

#include "core/inf/builder.h"

#include "graphics/inf/layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_elements.h"

namespace ark {

class PointLayer : public Layer {
public:
    PointLayer(const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(const LayerContext& renderContext, RenderCommandPipeline& pipeline, float x, float y) override;

//  [[plugin::resource-loader("point-layer")]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Atlas>> _atlas;
        sp<Builder<GLShader>> _shader;
    };

private:
    GLElements _elements;
};

}

#endif
