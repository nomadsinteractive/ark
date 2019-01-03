#ifndef ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_SPHERE_H_
#define ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_SPHERE_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/render_model.h"

namespace ark {

class GLModelSphere : public RenderModel {
public:
    GLModelSphere(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Atlas>& atlas, uint32_t sampleCount);

    virtual sp<ShaderBindings> makeShaderBindings(const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout) override;

    virtual void start(ModelBuffer& buf, RenderController& renderController, const Layer::Snapshot& layerContext) override;
    virtual void load(ModelBuffer& buf, int32_t type, const V& size) override;

//  [[plugin::resource-loader("sphere")]]
    class BUILDER : public Builder<RenderModel> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<RenderModel> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
        uint32_t _sample_count;

        sp<ResourceLoaderContext> _resource_loader_context;
    };

private:
    void buildVertex(float*& buffer, float lng, float lat) const;
    void buildTexture(float*& buffer, float lng, float lat) const;
    void degenerate(glindex_t*& buffer, glindex_t index) const;

private:
    sp<Atlas> _atlas;

    uint32_t _vertex_count;
    floatarray _vertices_boiler_plate;
    indexarray _indices_boiler_plate;

    Buffer _instance_index;
};

}

#endif
