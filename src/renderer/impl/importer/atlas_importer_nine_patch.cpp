#include "renderer/impl/importer/atlas_importer_nine_patch.h"

#include "core/util/documents.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture.h"
#include "renderer/impl/vertices/vertices_nine_patch.h"

namespace ark {

AtlasImporterNinePatch::BUILDER::BUILDER(const document& manifest)
    : _manifest(manifest)
{
}

sp<AtlasImporter> AtlasImporterNinePatch::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporterNinePatch>::make(_manifest);
}

AtlasImporterNinePatch::AtlasImporterNinePatch(document manifest)
    : _manifest(std::move(manifest))
{
}

void AtlasImporterNinePatch::import(Atlas& atlas, const sp<Readable>& /*readable*/)
{
    const sp<Attachment>& vertices = atlas.attachments().ensure<Attachment>();
    vertices->import(atlas, _manifest);
}

void AtlasImporterNinePatch::Attachment::import(Atlas& atlas, const document& manifest)
{
    uint32_t textureWidth = atlas.width();
    uint32_t textureHeight = atlas.height();
    for(const document& i : manifest->children())
    {
        const String name = i->name();
        const Rect paddings = Documents::ensureAttribute<Rect>(i, Constants::Attributes::NINE_PATCH_PADDINGS);
        if(name == "default")
        {
            for(const auto& i : atlas.items())
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

void AtlasImporterNinePatch::Attachment::add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Atlas& atlas)
{
    const Atlas::Item& item = atlas.at(type);
    const Rect bounds(item.ux() * textureWidth / 65536.0f, item.vy() * textureHeight / 65536.0f,
                      item.vx() * textureWidth / 65536.0f, item.uy() * textureHeight / 65536.0f);
    add(type, textureWidth, textureHeight, paddings, bounds);
}

void AtlasImporterNinePatch::Attachment::add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds)
{
    _vertices[type] = makeNinePatchVertices(textureWidth, textureHeight, paddings, bounds);
}

sp<Vertices> AtlasImporterNinePatch::Attachment::makeNinePatchVertices(uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds) const
{
    const Rect patches(paddings.left(), paddings.top(), bounds.width() - paddings.right(), bounds.height() - paddings.bottom());
    return sp<VerticesNinePatch>::make(bounds, patches, textureWidth, textureHeight);
}

const sp<Vertices>& AtlasImporterNinePatch::Attachment::ensureVertices(int32_t type) const
{
    const auto iter = _vertices.find(type);
    DCHECK(iter != _vertices.end(), "Cannot find type: %d", type);
    return iter->second;
}

}
