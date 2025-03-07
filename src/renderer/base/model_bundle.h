#pragma once

#include "core/base/bean_factory.h"
#include "core/base/manifest.h"
#include "core/collection/table.h"
#include "core/inf/builder.h"
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

        Vector<MeshLayout> _mesh_layouts;
        Vector<NodeLayout> _node_layouts;
    };
    struct Stub;

public:
    ModelBundle(sp<Importer> importer, sp<MaterialBundle> materialBundle = nullptr);
    ModelBundle(const sp<Stub>& stub);

    void import(BeanFactory& factory, const document& manifest, const Scope& args);

    sp<DrawingContextComposer> makeRenderCommandComposer(const Shader& shader) override;
    sp<Model> loadModel(int32_t type) override;

    const ModelLayout& ensureModelLayout(int32_t type) const;

//  [[script::bindings::property]]
    const sp<MaterialBundle>& materialBundle() const;

//  [[script::bindings::auto]]
    sp<Model> getModel(const NamedHash& type) const;

//  [[script::bindings::auto]]
    void importModel(const NamedHash& type, const String& manifest, sp<Future> future = nullptr);
//  [[script::bindings::auto]]
    void importModel(const NamedHash& type, const Manifest& manifest, sp<Future> future = nullptr);

//  [[script::bindings::auto]]
    void importMaterials(const NamedHash& type, const String& manifest);
//  [[script::bindings::auto]]
    void importMaterials(const NamedHash& type, const Manifest& manifest);

//  [[script::bindings::property]]
    size_t vertexLength() const;
//  [[script::bindings::property]]
    size_t indexLength() const;

    const Table<int32_t, ModelLayout>& modelLayouts() const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<ModelBundle> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<ModelBundle> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        document _manifest;

        SafeBuilder<MaterialBundle> _material_bundle;
        sp<Builder<Importer>> _importer;
    };

//  [[plugin::builder("model-bundle")]]
    class MODEL_LOADER_BUILDER : public Builder<ModelLoader> {
    public:
        MODEL_LOADER_BUILDER(BeanFactory& factory, const document& manifest);

        sp<ModelLoader> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

private:
    sp<Stub> _stub;
};

}
