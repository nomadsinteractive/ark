#include "renderer/impl/model_loader/model_loader_nine_patch.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements.h"
#include "renderer/impl/vertices/vertices_nine_patch.h"
#include "renderer/util/render_util.h"

namespace ark {

ModelLoaderNinePatch::ModelLoaderNinePatch(const document& manifest, const sp<Atlas>& atlas)
    : ModelLoader(ModelLoader::RENDER_MODE_TRIANGLE_STRIP), _atlas(atlas), _vertices(atlas->attachments().ensure<NinePatchVertices>())
{
    _vertices->import(atlas, manifest);
}

sp<RenderCommandComposer> ModelLoaderNinePatch::makeRenderCommandComposer()
{
    return sp<RCCDrawElements>::make(RenderUtil::makeUnitNinePatchModel());
}

void ModelLoaderNinePatch::initialize(ShaderBindings& shaderBindings)
{
    shaderBindings.pipelineBindings()->bindSampler(_atlas->texture());
}

void ModelLoaderNinePatch::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

Model ModelLoaderNinePatch::loadModel(int32_t type)
{
    const auto iter = _vertices->_vertices.find(type);
    DCHECK(iter != _vertices->_vertices.end(), "Cannot find type: %d", type);
    return Model(nullptr, iter->second);
}

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

void ModelLoaderNinePatch::NinePatchAtlasImporter::import(Atlas& atlas, BeanFactory& /*factory*/, const document& manifest)
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
        const String name = i->name();
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
    _vertices[type] = makeNinePatchVertices(textureWidth, textureHeight, paddings, bounds);
}

sp<Vertices> ModelLoaderNinePatch::NinePatchVertices::makeNinePatchVertices(uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds) const
{
    const Rect patches(paddings.left(), paddings.top(), bounds.width() - paddings.right(), bounds.height() - paddings.bottom());
    return sp<VerticesNinePatch>::make(bounds, patches, textureWidth, textureHeight);
}

}
