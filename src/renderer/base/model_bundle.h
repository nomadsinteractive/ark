#pragma once

#include "core/base/bean_factory.h"
#include "core/base/manifest.h"
#include "core/collection/table.h"
#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

namespace ark {

//[[script::bindings::extends(ModelLoader)]]
class ARK_API ModelBundle final : public ModelLoader {
public:
    struct MeshLayout {
        sp<Mesh> _mesh;
        size_t _index_offset;
        size_t _vertex_offset;
    };

    struct NodeLayout {
        NodeLayout() = default;
        NodeLayout(const sp<Node>& node, const NodeLayout& parentLayout);

        sp<Node> _node;
        M4 _transform;
    };

    struct ModelLayout {
        sp<Model> _model;
        size_t _vertex_offset;
        size_t _index_offset;

        std::vector<MeshLayout> _mesh_layouts;
        std::vector<NodeLayout> _node_layouts;
    };

private:
    struct Stub {
        Stub(sp<MaterialBundle> materialBundle, sp<Importer> importer);

        void import(BeanFactory& factory, const document& manifest, const Scope& args);

        sp<Model> importModel(const Manifest& manifest, const sp<Importer>& importer);
        ModelLayout& addModel(int32_t type, sp<Model> model);
        const ModelLayout& ensureModelLayout(int32_t type) const;

        sp<MaterialBundle> _material_bundle;
        sp<Importer> _importer;

        Table<int32_t, ModelLayout> _model_layouts;
        size_t _vertex_length;
        size_t _index_length;
    };

    class AddModuleRunnable : public Runnable {
    public:
        AddModuleRunnable(int32_t type, sp<Stub> stub, sp<Model> model, sp<Future> future);

        void run() override;

    private:
        int32_t _type;
        sp<Stub> _stub;
        sp<Model> _model;
        sp<Future> _future;

    };

    class ImportModuleRunnable : public Runnable {
    public:
        ImportModuleRunnable(int32_t type, Manifest manifest, const sp<Stub>& stub, sp<Importer> importer, sp<Executor> executor, sp<Future> future);

        void run() override;

    private:
        int32_t _type;
        Manifest _manifest;
        sp<Stub> _stub;
        sp<Importer> _importer;
        sp<Executor> _executor;
        sp<Future> _future;
    };

public:
    ModelBundle(sp<MaterialBundle> materialBundle, sp<Importer> importer);
    ModelBundle(const sp<Stub>& stub);

    void import(BeanFactory& factory, const document& manifest, const Scope& args);

    sp<RenderCommandComposer> makeRenderCommandComposer(const Shader& shader) override;
    sp<Model> loadModel(int32_t type) override;

    const ModelLayout& ensureModelLayout(int32_t type) const;

//[[script::bindings::auto]]
    sp<Model> getModel(const NamedType& namedType) const;

//[[script::bindings::auto]]
    void importModel(const NamedType& namedType, const String& src, sp<Future> future = nullptr);
//[[script::bindings::auto]]
    void importModel(const NamedType& namedType, const Manifest& manifest, sp<Future> future = nullptr);

//[[script::bindings::property]]
    size_t vertexLength() const;
//[[script::bindings::property]]
    size_t indexLength() const;

    const Table<int32_t, ModelLayout>& modelLayouts() const;

//  [[plugin::builder]]
    class BUILDER : public Builder<ModelBundle> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ModelBundle> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        document _manifest;

        SafePtr<Builder<MaterialBundle>> _material_bundle;
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
