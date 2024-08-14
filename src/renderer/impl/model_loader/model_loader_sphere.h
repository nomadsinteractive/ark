#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/base/model_bundle.h"

namespace ark {

class ModelLoaderSphere : public ModelLoader {
public:
[[deprecated]]
    ModelLoaderSphere(const sp<Atlas>& atlas, uint32_t sampleCount);

    sp<RenderCommandComposer> makeRenderCommandComposer(const Shader& shader) override;
    sp<Model> loadModel(int32_t type) override;

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

    sp<Uploader> _indices;
    sp<std::vector<ModelLoaderSphere::Vertex>> _vertices;

};

}
