#include "renderer/base/model_bundle.h"

#include "core/base/bean_factory.h"
#include "core/util/conversions.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture_packer.h"
#include "renderer/inf/vertices.h"
#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"


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

const ModelBundle::ModelInfo& ModelBundle::ensure(int32_t type) const
{
    return _stub->ensure(type);
}

sp<Model> ModelBundle::loadModel(int32_t type)
{
    return ensure(type)._model;
}

sp<Model> ModelBundle::load(int32_t type)
{
    return loadModel(type);
}

const Table<int32_t, ModelBundle::ModelInfo>& ModelBundle::models() const
{
    return _stub->_models;
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
        addModel(type, importer ? factory.build<Importer>(importer, args)->import(i, _material_bundle) : _importer->import(i, _material_bundle));
    }
}

ModelBundle::ModelInfo& ModelBundle::Stub::addModel(int32_t type, const Model& model)
{
    ModelInfo& modelInfo = _models[type];
    modelInfo = {sp<Model>::make(model), _vertex_length, _index_length};
    _vertex_length += model.vertexCount();
    _index_length += model.indexCount();
    return modelInfo;
}

const ModelBundle::ModelInfo& ModelBundle::Stub::ensure(int32_t type) const
{
    const auto iter = _models.find(type);
    CHECK(iter != _models.end(), "Model not found, type: %d", type);
    return iter->second;
}

ModelBundle::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _bean_factory(factory), _manifest(manifest), _material_bundle(factory.ensureBuilder<MaterialBundle>(manifest, "material-bundle")),
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
    : _material_bundle(std::move(materialBundle)), _importer(std::move(importer)), _vertex_length(0), _index_length(0)
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

}
