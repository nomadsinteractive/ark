#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_SPHERE_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_SPHERE_H_

#include "core/types/shared_ptr.h"

#include "renderer/base/model_bundle.h"

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

//  [[plugin::builder::by-value("sphere")]]
    class IMPORTER_BUILDER : public Builder<ModelLoader::Importer> {
    public:
        IMPORTER_BUILDER();

        virtual sp<ModelLoader::Importer> build(const Scope& args) override;

    };

    struct Vertex {
        V3 _position;
        V3 _tangent;
        float _u, _v;
    };

private:
    sp<Atlas> _atlas;

    indexarray _indices;
    sp<std::vector<ModelLoaderSphere::Vertex>> _vertices;

};

}

#endif
