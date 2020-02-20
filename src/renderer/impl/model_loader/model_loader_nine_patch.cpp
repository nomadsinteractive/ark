#include "renderer/impl/model_loader/model_loader_nine_patch.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/impl/render_command_composer/rcc_uniform_models.h"
#include "renderer/impl/vertices/vertices_nine_patch.h"
#include "renderer/util/element_util.h"

namespace ark {

ModelLoaderNinePatch::ModelLoaderNinePatch(const document& manifest, const sp<Atlas>& atlas)
    : ModelLoader(ModelLoader::RENDER_MODE_TRIANGLE_STRIP), _atlas(atlas), _vertices(atlas->attachments().ensure<NinePatchVertices>())
{
//    uint32_t textureWidth = static_cast<uint32_t>(_atlas->texture()->width());
//    uint32_t textureHeight = static_cast<uint32_t>(_atlas->texture()->height());
//    for(const document& node : manifest->children())
//    {
//        String name = node->name();
//        const Rect paddings = Documents::ensureAttribute<Rect>(node, Constants::Attributes::NINE_PATCH_PADDINGS);
//        if(name == "import")
//        {
//            for(const auto& i : _atlas->items()->indices())
//                importAtlasItem(i.first, paddings, textureWidth, textureHeight);
//        }
//        else
//        {
//            int32_t type = Documents::getAttribute<int32_t>(node, Constants::Attributes::TYPE, 0);
//            bool hasBounds = atlas->has(type);
//            if(hasBounds)
//                importAtlasItem(type, paddings, textureWidth, textureHeight);
//            else
//            {
//                const Rect bounds = Rect::parse(node);
//                _vertices.insert({type, sp<VerticesNinePatch>::make(bounds, getPatches(paddings, bounds), textureWidth, textureHeight)});
//            }
//        }
//    }
    _vertices->import(atlas, manifest);
}

sp<RenderCommandComposer> ModelLoaderNinePatch::makeRenderCommandComposer()
{
    return sp<RCCUniformModels>::make(ElementUtil::makeUnitNinePatchModel());
}

void ModelLoaderNinePatch::initialize(ShaderBindings& shaderBindings)
{
    shaderBindings.pipelineBindings()->bindSampler(_atlas->texture());
}

void ModelLoaderNinePatch::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

Model ModelLoaderNinePatch::load(int32_t type)
{
    const auto iter = _vertices->_vertices.find(type);
    DCHECK(iter != _vertices->_vertices.end(), "Cannot find type: %d", type);
    return Model(nullptr, iter->second);
}

//void ModelLoaderNinePatch::importAtlasItem(int32_t type, const Rect& paddings, uint32_t textureWidth, uint32_t textureHeight)
//{
//    const Atlas::Item& item = _atlas->at(type);
//    const Rect bounds(item.ux() * textureWidth / 65536.0f, item.vy() * textureHeight / 65536.0f,
//                      item.vx() * textureWidth / 65536.0f, item.uy() * textureHeight / 65536.0f);
//    _vertices.insert({type, sp<VerticesNinePatch>::make(bounds, getPatches(paddings, bounds), textureWidth, textureHeight)});
//}

//Rect ModelLoaderNinePatch::getPatches(const Rect& paddings, const Rect& bounds) const
//{
//    return Rect(paddings.left(), paddings.top(), bounds.width() - paddings.right(), bounds.height() - paddings.bottom());
//}

ModelLoaderNinePatch::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _manifest(manifest), _atlas(factory.ensureConcreteClassBuilder<Atlas>(manifest, Constants::Attributes::ATLAS))
{
}

sp<ModelLoader> ModelLoaderNinePatch::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderNinePatch>::make(_manifest, _atlas->build(args));
}

sp<Atlas::Importer> ModelLoaderNinePatch::ATLAS_IMPORTER_BUILDER::build(const Scope& /*args*/)
{
    return sp<NinePatchAtlasImporter>::make();
}

void ModelLoaderNinePatch::NinePatchAtlasImporter::import(Atlas& atlas, const document& manifest)
{
    const sp<NinePatchVertices>& vertices = atlas.attachments().ensure<NinePatchVertices>();
    vertices->import(atlas, manifest);
}

void ModelLoaderNinePatch::NinePatchVertices::import(Atlas& atlas, const document& manifest)
{
    uint32_t textureWidth = static_cast<uint32_t>(atlas.texture()->width());
    uint32_t textureHeight = static_cast<uint32_t>(atlas.texture()->height());
    for(const document& i : manifest->children())
    {
        String name = i->name();
        const Rect paddings = Documents::ensureAttribute<Rect>(i, Constants::Attributes::NINE_PATCH_PADDINGS);
        if(name == "default")
        {
            for(const auto& i : atlas.items()->indices())
                add(i.first, textureWidth, textureHeight, paddings, atlas);
        }
        else
        {
            DWARN(name == "nine-patch", "\"%s\" nodeName should be \"nine-patch\"", Documents::toString(i).c_str());
            int32_t type = Documents::getAttribute<int32_t>(i, Constants::Attributes::TYPE, 0);
            bool hasBounds = atlas.has(type);
            if(hasBounds)
                add(type, textureWidth, textureHeight, paddings, atlas);
            else
            {
                const Rect bounds = Rect::parse(i);
                add(type, textureWidth, textureHeight, paddings, bounds);
            }
        }
    }
}

void ModelLoaderNinePatch::NinePatchVertices::add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Atlas& atlas)
{
    const Atlas::Item& item = atlas.at(type);
    const Rect bounds(item.ux() * textureWidth / 65536.0f, item.vy() * textureHeight / 65536.0f,
                      item.vx() * textureWidth / 65536.0f, item.uy() * textureHeight / 65536.0f);
    add(type, textureWidth, textureHeight, paddings, bounds);
}

void ModelLoaderNinePatch::NinePatchVertices::add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds)
{
    _vertices.insert({type, sp<VerticesNinePatch>::make(bounds, getPatches(paddings, bounds), textureWidth, textureHeight)});
}

Rect ModelLoaderNinePatch::NinePatchVertices::getPatches(const Rect& paddings, const Rect& bounds) const
{
    return Rect(paddings.left(), paddings.top(), bounds.width() - paddings.right(), bounds.height() - paddings.bottom());
}

}
