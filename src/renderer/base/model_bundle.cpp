#include "renderer/base/model_bundle.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/future.h"
#include "core/base/named_hash.h"
#include "core/inf/executor.h"
#include "core/impl/executor/executor_thread_pool.h"

#include "renderer/base/material_bundle.h"
#include "renderer/base/mesh.h"
#include "renderer/base/node.h"
#include "renderer/base/texture_packer.h"
#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"

#include "app/base/application_context.h"
#include "graphics/base/material.h"

namespace ark {

ModelBundle::ModelBundle(sp<Importer> importer, sp<MaterialBundle> materialBundle)
    : ModelLoader(Enum::RENDER_MODE_TRIANGLES, nullptr), _stub(sp<Stub>::make(std::move(importer), std::move(materialBundle)))
{
}

ModelBundle::ModelBundle(const sp<ModelBundle::Stub>& stub)
    : ModelLoader(Enum::RENDER_MODE_TRIANGLES, nullptr), _stub(stub)
{
}

void ModelBundle::import(BeanFactory& factory, const document& manifest, const Scope& args)
{
    _stub->import(factory, manifest, args);
}

sp<RenderCommandComposer> ModelBundle::makeRenderCommandComposer(const Shader& /*shader*/)
{
    return sp<RenderCommandComposer>::make<RCCMultiDrawElementsIndirect>(sp<ModelBundle>::make(_stub));
}

const ModelBundle::ModelLayout& ModelBundle::ensureModelLayout(int32_t type) const
{
    return _stub->ensureModelLayout(type);
}

const sp<MaterialBundle>& ModelBundle::materialBundle() const
{
    return _stub->_material_bundle;
}

sp<Model> ModelBundle::getModel(const NamedHash& type) const
{
    const auto iter = _stub->_model_layouts.find(type.hash());
    return iter != _stub->_model_layouts.end() ? iter->second._model : nullptr;
}

sp<Model> ModelBundle::loadModel(int32_t type)
{
    return ensureModelLayout(type)._model;
}

void ModelBundle::importModel(const NamedHash& type, const String& manifest, sp<Future> future)
{
    importModel(type.hash(), Manifest(manifest), std::move(future));
}

void ModelBundle::importModel(const NamedHash& type, const Manifest& manifest, sp<Future> future)
{
    const ApplicationContext& applicationContext = Ark::instance().applicationContext();
    sp<Runnable> task = sp<Runnable>::make<ImportModuleRunnable>(type.hash(), manifest, _stub, nullptr, applicationContext.executorMain(), std::move(future));
    applicationContext.executorThreadPool()->execute(std::move(task));
}

void ModelBundle::importMaterials(const NamedHash& type, const String& manifest)
{
    importMaterials(type, Manifest(manifest));
}

void ModelBundle::importMaterials(const NamedHash& type, const Manifest& manifest)
{
    sp<Model> model = _stub->importModel(manifest, nullptr);
    for(const sp<Material>& i : model->materials())
        _stub->_material_bundle->addMaterial(i->name(), i);
    if(type)
        _stub->addModel(type.hash(), std::move(model));
}

const Table<int32_t, ModelBundle::ModelLayout>& ModelBundle::modelLayouts() const
{
    return _stub->_model_layouts;
}

size_t ModelBundle::vertexLength() const
{
    return _stub->_vertex_length;
}

size_t ModelBundle::indexLength() const
{
    return _stub->_index_length;
}

void ModelBundle::Stub::import(BeanFactory& factory, const document& manifest, const Scope& args)
{
    for(const document& i : manifest->children("model"))
    {
        const int32_t type = Documents::ensureAttribute<int32_t>(i, constants::TYPE);
        const String importer = Documents::getAttribute(i, "importer");
        const Manifest manifest(Documents::getAttribute(i, constants::SRC, ""), i);
        addModel(type, importModel(manifest, importer ? factory.build<Importer>(importer, args) : nullptr));
    }
}

sp<Model> ModelBundle::Stub::importModel(const Manifest& manifest, const sp<Importer>& importer)
{
    return sp<Model>::make((importer ? importer : _importer)->import(manifest, _material_bundle));
}

ModelBundle::ModelLayout& ModelBundle::Stub::addModel(const int32_t type, sp<Model> model)
{
    CHECK(_model_layouts.find(type) == _model_layouts.end(), "Model[%d] exists already", type);
    ModelLayout& modelLayout = _model_layouts[type];
    modelLayout._node_layouts = model->toFlatLayouts<NodeLayout>();
    modelLayout._vertex_offset = _vertex_length;
    modelLayout._index_offset = _index_length;
    size_t meshIndexOffset = _index_length;
    size_t meshVertexOffset = _vertex_length;
    for(const sp<Mesh>& i : model->meshes())
    {
        MeshLayout ml = {i, meshIndexOffset, meshVertexOffset};
        meshIndexOffset += i->indices().size();
        meshVertexOffset += i->vertexCount();
        modelLayout._mesh_layouts.push_back(std::move(ml));
    }
    _vertex_length += model->vertexCount();
    _index_length += model->indexCount();

    modelLayout._model = std::move(model);
    return modelLayout;
}

const ModelBundle::ModelLayout& ModelBundle::Stub::ensureModelLayout(int32_t type) const
{
    const auto iter = _model_layouts.find(type);
    CHECK(iter != _model_layouts.end(), "Model not found, type: %d(%s)", type, NamedHash::reverse(type).c_str());
    return iter->second;
}

ModelBundle::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _bean_factory(factory), _manifest(manifest), _material_bundle(factory.getBuilder<MaterialBundle>(manifest, "material-bundle")),
      _importer(factory.ensureBuilder<ModelLoader::Importer>(manifest, "importer"))
{
}

sp<ModelBundle> ModelBundle::BUILDER::build(const Scope& args)
{
    sp<ModelBundle> modelBundle = sp<ModelBundle>::make(_importer->build(args), _material_bundle.build(args));
    modelBundle->import(_bean_factory, _manifest, args);
    return modelBundle;
}

ModelBundle::Stub::Stub(sp<ModelLoader::Importer> importer, sp<MaterialBundle> materialBundle)
    : _importer(std::move(importer)), _material_bundle(materialBundle ? std::move(materialBundle) : sp<MaterialBundle>::make()), _vertex_length(0), _index_length(0)
{
}

ModelBundle::MODEL_LOADER_BUILDER::MODEL_LOADER_BUILDER(BeanFactory& factory, const document& manifest)
    : _impl(factory, manifest)
{
}

sp<ModelLoader> ModelBundle::MODEL_LOADER_BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

ModelBundle::ImportModuleRunnable::ImportModuleRunnable(int32_t type, Manifest manifest, const sp<Stub>& stub, sp<Importer> importer, sp<Executor> executor, sp<Future> future)
    : _type(type), _manifest(std::move(manifest)), _stub(stub), _importer(std::move(importer)), _executor(std::move(executor)), _future(std::move(future))
{
}

void ModelBundle::ImportModuleRunnable::run()
{
    _executor->execute(sp<Runnable>::make<AddModuleRunnable>(_type, std::move(_stub), _stub->importModel(_manifest, _importer), std::move(_future)));
}

ModelBundle::AddModuleRunnable::AddModuleRunnable(int32_t type, sp<Stub> stub, sp<Model> model, sp<Future> future)
    : _type(type), _stub(std::move(stub)), _model(std::move(model)), _future(std::move(future))
{
}

void ModelBundle::AddModuleRunnable::run()
{
    _stub->addModel(_type, std::move(_model));
    if(_future)
        _future->done();
}

ModelBundle::NodeLayout::NodeLayout(const sp<Node>& node, const NodeLayout& parentLayout)
    : _node(node), _transform(parentLayout._node ? parentLayout._transform * _node->localMatrix() : _node->localMatrix())
{
}

}
