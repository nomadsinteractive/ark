#include "renderer/base/model_bundle.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/future.h"
#include "core/inf/executor.h"
#include "core/impl/executor/executor_thread_pool.h"
#include "core/util/string_convert.h"

#include "renderer/base/atlas.h"
#include "renderer/base/material_bundle.h"
#include "renderer/base/mesh.h"
#include "renderer/base/node.h"
#include "renderer/base/texture_packer.h"
#include "renderer/inf/vertices.h"
#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"

#include "app/base/application_context.h"

namespace ark {

ModelBundle::ModelBundle(sp<MaterialBundle> materialBundle, sp<Importer> importer)
    : ModelLoader(RENDER_MODE_TRIANGLES), _stub(sp<Stub>::make(std::move(materialBundle), std::move(importer)))
{
}

ModelBundle::ModelBundle(const sp<ModelBundle::Stub>& stub)
    : ModelLoader(RENDER_MODE_TRIANGLES), _stub(stub)
{
}

void ModelBundle::import(BeanFactory& factory, const document& manifest, const Scope& args)
{
    _stub->import(factory, manifest, args);
}

sp<RenderCommandComposer> ModelBundle::makeRenderCommandComposer()
{
    return sp<RCCMultiDrawElementsIndirect>::make(sp<ModelBundle>::make(_stub));
}

void ModelBundle::initialize(ShaderBindings& /*shaderBindings*/)
{
}

const ModelBundle::ModelLayout& ModelBundle::ensureModelLayout(int32_t type) const
{
    return _stub->ensureModelLayout(type);
}

sp<Model> ModelBundle::loadModel(int32_t type)
{
    return ensureModelLayout(type)._model;
}

sp<Model> ModelBundle::getModel(int32_t type)
{
    const auto iter = _stub->_model_layouts.find(type);
    return iter != _stub->_model_layouts.end() ? iter->second._model : nullptr;
}

void ModelBundle::importModel(int32_t type, const String& src, sp<Future> future)
{
    importModel(type, Manifest(src), std::move(future));
}

void ModelBundle::importModel(int32_t type, const Manifest& manifest, sp<Future> future)
{
    ApplicationContext& applicationContext = Ark::instance().applicationContext();
    sp<Runnable> task = sp<ImportModuleRunnable>::make(type, manifest, _stub, nullptr, applicationContext.executorMain(), std::move(future));
    applicationContext.executorThreadPool()->execute(task);
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
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
        const String importer = Documents::getAttribute(i, "importer");
        const Manifest manifest(Documents::getAttribute(i, Constants::Attributes::SRC, ""), i);
        addModel(type, importModel(manifest, importer ? factory.build<Importer>(importer, args) : nullptr));
    }
}

sp<Model> ModelBundle::Stub::importModel(const Manifest& manifest, const sp<Importer>& importer)
{
    return sp<Model>::make((importer ? importer : _importer)->import(manifest, _material_bundle));
}

ModelBundle::ModelLayout& ModelBundle::Stub::addModel(int32_t type, sp<Model> model)
{
    ModelLayout& modelInfo = _model_layouts[type];
    modelInfo._node_layouts = model->toFlatLayouts<NodeLayout>();
    modelInfo._vertex_offset = _vertex_length;
    modelInfo._index_offset = _index_length;
    size_t meshIndexOffset = _index_length;
    for(const sp<Mesh>& i : model->meshes())
    {
        MeshLayout ml = {i, meshIndexOffset};
        meshIndexOffset += i->indices().size();
        modelInfo._mesh_layouts.push_back(std::move(ml));
    }
    _vertex_length += model->vertexCount();
    _index_length += model->indexCount();

    modelInfo._model = std::move(model);
    return modelInfo;
}

const ModelBundle::ModelLayout& ModelBundle::Stub::ensureModelLayout(int32_t type) const
{
    const auto iter = _model_layouts.find(type);
    CHECK(iter != _model_layouts.end(), "Model not found, type: %d", type);
    return iter->second;
}

ModelBundle::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _bean_factory(factory), _manifest(manifest), _material_bundle(factory.getBuilder<MaterialBundle>(manifest, "material-bundle")),
      _importer(factory.ensureBuilder<ModelLoader::Importer>(manifest, "importer"))
{
}

sp<ModelBundle> ModelBundle::BUILDER::build(const Scope& args)
{
    sp<ModelBundle> modelBundle = sp<ModelBundle>::make(_material_bundle->build(args), _importer->build(args));
    modelBundle->import(_bean_factory, _manifest, args);
    return modelBundle;
}

ModelBundle::Stub::Stub(sp<MaterialBundle> materialBundle, sp<ModelLoader::Importer> importer)
    : _material_bundle(materialBundle ? std::move(materialBundle) : sp<MaterialBundle>::make()), _importer(std::move(importer)), _vertex_length(0), _index_length(0)
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
    : _type(type), _manifest(std::move(manifest)), _stub(stub), _importer(std::move(importer)), _executor(std::move(executor)), _future(future)
{
}

void ModelBundle::ImportModuleRunnable::run()
{
    _executor->execute(sp<AddModuleRunnable>::make(_type, std::move(_stub), _stub->importModel(_manifest, _importer), std::move(_future)));
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
    : _node(node), _transform(parentLayout._node ? parentLayout._transform * _node->transform() : _node->transform())
{
}

}
