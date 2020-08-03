#include "renderer/base/model_bundle.h"

#include "core/base/bean_factory.h"
#include "core/util/conversions.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture_packer.h"
#include "renderer/inf/vertices.h"
#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"


namespace ark {

ModelBundle::ModelBundle(sp<Atlas> atlas, sp<Importer> importer)
    : ModelLoader(RENDER_MODE_TRIANGLES), _stub(sp<Stub>::make(std::move(atlas), std::move(importer)))
{
}

ModelBundle::ModelBundle(const sp<ModelBundle::Stub>& stub)
    : ModelLoader(RENDER_MODE_TRIANGLES), _stub(stub)
{
}

void ModelBundle::import(const sp<ResourceLoaderContext>& resourceLoaderContext, BeanFactory& factory, const document& manifest, const Scope& args)
{
    _stub->import(resourceLoaderContext, factory, manifest, args);
}

sp<RenderCommandComposer> ModelBundle::makeRenderCommandComposer()
{
    return sp<RCCMultiDrawElementsIndirect>::make(sp<ModelBundle>::make(_stub));
}

void ModelBundle::initialize(ShaderBindings& /*shaderBindings*/)
{
}

void ModelBundle::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

const ModelBundle::ModelInfo& ModelBundle::ensure(int32_t type) const
{
    return _stub->ensure(type);
}

Model ModelBundle::loadModel(int32_t type)
{
    return ensure(type)._model;
}

sp<Model> ModelBundle::load(int32_t type)
{
    return sp<Model>::make(loadModel(type));
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

void ModelBundle::Stub::import(const sp<ResourceLoaderContext>& resourceLoaderContext, BeanFactory& factory, const document& manifest, const Scope& args)
{
    bool hasModelMaps = false;
    TexturePacker texturePacker(resourceLoaderContext, static_cast<int32_t>(_atlas->width()), static_cast<int32_t>(_atlas->height()), false);
    for(const document& i : manifest->children())
    {
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
        for(const document& j : i->children("map"))
        {
            const String& mappingSrc = Documents::ensureAttribute(j, Constants::Attributes::SRC);
            const RectI rect = texturePacker.addBitmap(mappingSrc);
            _atlas->add(type, rect.left(), rect.top(), rect.right(), rect.bottom(), Rect(0, 0, 1.0f, 1.0f), V2(rect.width(), rect.height()), V2(0.5f, 0.5f));
            hasModelMaps = true;
        }

        const Rect bounds = _atlas && _atlas->has(type) ? _atlas->getItemUV(type) : Rect(0, 1.0f, 1.0f, 0);

        const String importer = Documents::getAttribute(i, "importer");
        addModel(type, importer ? factory.build<Importer>(importer, args)->import(i, bounds) : _importer->import(i, bounds));
    }
    if(hasModelMaps)
        texturePacker.updateTexture(_atlas->texture());
}

ModelBundle::ModelInfo& ModelBundle::Stub::addModel(int32_t type, const Model& model)
{
    ModelInfo& modelInfo = _models[type];
    modelInfo = {model, _vertex_length, _index_length};
    _vertex_length += model.vertexLength();
    _index_length += model.indexLength();
    return modelInfo;
}

const ModelBundle::ModelInfo& ModelBundle::Stub::ensure(int32_t type) const
{
    const auto iter = _models.find(type);
    DCHECK(iter != _models.end(), "Model not found, type: %d", type);
    return iter->second;
}

template<> ARK_API ModelBundle::MappingType Conversions::to<String, ModelBundle::MappingType>(const String& str)
{
    DCHECK(str == "albedo", "Only albedo mapping is supported");
    return ModelBundle::MAPPING_TYPE_ALBEDO;
}

ModelBundle::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _bean_factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _atlas(factory.ensureBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)),
      _importer(factory.ensureBuilder<ModelLoader::Importer>(manifest, "importer"))
{
}

sp<ModelBundle> ModelBundle::BUILDER::build(const Scope& args)
{
    sp<Importer> importer = _importer->build(args);
    sp<ModelBundle> modelBundle = sp<ModelBundle>::make(_atlas->build(args), std::move(importer));
    modelBundle->import(_resource_loader_context, _bean_factory, _manifest, args);
    return modelBundle;
}

ModelBundle::Stub::Stub(sp<Atlas> atlas, sp<ModelLoader::Importer> importer)
    : _atlas(std::move(atlas)), _importer(std::move(importer)), _vertex_length(0), _index_length(0)
{
}

ModelBundle::MODEL_LOADER_BUILDER::MODEL_LOADER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext)
{
}

sp<ModelLoader> ModelBundle::MODEL_LOADER_BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
