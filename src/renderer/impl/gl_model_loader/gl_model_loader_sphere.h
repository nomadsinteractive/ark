#ifndef ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_SPHERE_H_
#define ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_SPHERE_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_model_loader.h"

namespace ark {

class GLModelLoaderSphere : public GLModelLoader {
public:
    GLModelLoaderSphere(const sp<Atlas>& atlas, uint32_t sampleCount);

    virtual void start(GLModelBuffer& buf, GLResourceManager& resourceManager, const LayerContext::Snapshot& layerContext) override;
    virtual void loadModel(GLModelBuffer& buf, const Atlas& atlas, uint32_t type, const V& size) override;

private:
    void buildVertex(float*& buffer, float lng, float lat) const;
    void buildTexture(float*& buffer, float lng, float lat) const;
    void degenerate(glindex_t*& buffer, glindex_t index) const;

private:
    sp<Atlas> _atlas;
    uint32_t _sample_count;
    uint32_t _vertex_count;
    floatarray _vertices_boiler_plate;
    indexarray _indices_boiler_plate;

    // GLModelLoader interface
public:
};

}

#endif
