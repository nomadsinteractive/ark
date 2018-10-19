#ifndef ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_SPHERE_H_
#define ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_SPHERE_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"
#include "renderer/inf/gl_model_loader.h"

namespace ark {

class GLModelLoaderSphere : public GLModelLoader {
public:
    GLModelLoaderSphere(const sp<ResourceLoaderContext>& resourceLoaderContext, uint32_t sampleCount);

    virtual void start(GLModelBuffer& buf, GLResourceManager& resourceManager, const Layer::Snapshot& layerContext) override;
    virtual void loadModel(GLModelBuffer& buf, const Atlas& atlas, int32_t type, const V& size) override;

//  [[plugin::resource-loader("sphere")]]
    class BUILDER : public Builder<GLModelLoader> {
    public:
        BUILDER(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLModelLoader> build(const sp<Scope>& args) override;

    private:
        uint32_t _sample_count;
        sp<ResourceLoaderContext> _resource_loader_context;
    };

private:
    void buildVertex(float*& buffer, float lng, float lat) const;
    void buildTexture(float*& buffer, float lng, float lat) const;
    void degenerate(glindex_t*& buffer, glindex_t index) const;

private:
    uint32_t _vertex_count;
    floatarray _vertices_boiler_plate;
    indexarray _indices_boiler_plate;

    GLBuffer _instance_index;
};

}

#endif
