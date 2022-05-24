#ifndef ARK_RENDERER_BASE_MODEL_BUNDLE_H_
#define ARK_RENDERER_BASE_MODEL_BUNDLE_H_

#include "core/base/bean_factory.h"
#include "core/collection/table.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ARK_API ModelBundle : public ModelLoader {
public:
    struct ModelInfo {
        sp<Model> _model;
        size_t _vertex_offset;
        size_t _index_offset;
    };

private:
    struct Stub {
        Stub(sp<MaterialBundle> materialBundle, sp<Importer> importer);

        void import(BeanFactory& factory, const document& manifest, const Scope& args);
        ModelInfo& addModel(int32_t type, const Model& model);
        const ModelInfo& ensure(int32_t type) const;

        sp<MaterialBundle> _material_bundle;
        sp<Importer> _importer;

        Table<int32_t, ModelInfo> _models;
        size_t _vertex_length;
        size_t _index_length;
    };

public:
    ModelBundle(sp<MaterialBundle> materialBundle, sp<Importer> importer);
    ModelBundle(const sp<Stub>& stub);

    void import(BeanFactory& factory, const document& manifest, const Scope& args);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;
    virtual void initialize(ShaderBindings& shaderBindings) override;
    virtual sp<Model> loadModel(int32_t type) override;

    const ModelInfo& ensure(int32_t type) const;

//[[script::bindings::auto]]
    sp<Model> load(int32_t type);
//[[script::bindings::property]]
    size_t vertexLength() const;
//[[script::bindings::property]]
    size_t indexLength() const;

    const Table<int32_t, ModelInfo>& models() const;

//  [[plugin::builder]]
    class BUILDER : public Builder<ModelBundle> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ModelBundle> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        document _manifest;

        sp<Builder<MaterialBundle>> _material_bundle;
        sp<Builder<Importer>> _importer;
    };

//  [[plugin::builder("model-bundle")]]
    class MODEL_LOADER_BUILDER : public Builder<ModelLoader> {
    public:
        MODEL_LOADER_BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

private:
    sp<Stub> _stub;

};

}

#endif
