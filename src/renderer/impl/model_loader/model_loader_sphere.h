#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_SPHERE_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_SPHERE_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderSphere : public ModelLoader {
public:
    ModelLoaderSphere(const sp<Atlas>& atlas, uint32_t sampleCount);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

    virtual void initialize(ShaderBindings& shaderBindings) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;

    virtual Model loadModel(int32_t type) override;

//  [[plugin::builder("sphere")]]
    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
        uint32_t _sample_count;
    };

private:
    struct Vertex {
        V3 _position;
        V3 _tangent;
        float _u, _v;
    };

private:
    sp<std::vector<ModelLoaderSphere::Vertex>> makeVertices();

    void buildVertex(Vertex& vertex, float lng, float lat) const;
    void buildTexture(Vertex& vertex, float lng, float lat) const;

    void degenerate(element_index_t*& buffer, element_index_t index) const;

private:
    sp<Atlas> _atlas;
    uint32_t _sample_count;

    indexarray _indices;
    std::unordered_map<int32_t, sp<std::vector<ModelLoaderSphere::Vertex>>> _vertices;

    friend class VerticesSphere;

};

}

#endif
