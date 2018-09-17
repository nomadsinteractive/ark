#include "renderer/impl/atlas_importer/atlas_importer_tiles.h"

#include "core/util/math.h"
#include "core/util/documents.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_texture.h"

namespace ark {

void AtlasImporterTiles::import(Atlas& atlas, const ResourceLoaderContext& /*resourceLoaderContext*/, const document& manifest)
{
    int32_t type = Documents::getAttribute<int32_t>(manifest, Constants::Attributes::TYPE, -1);
    const uint32_t tileWidth = Documents::ensureAttribute<uint32_t>(manifest, "tile-width");
    const uint32_t tileHeight = Documents::ensureAttribute<uint32_t>(manifest, "tile-height");
    const uint32_t marginX = Documents::getAttribute<uint32_t>(manifest, "margin-x", 0);
    const uint32_t marginY = Documents::getAttribute<uint32_t>(manifest, "margin-y", 0);
    const float pivotX = Documents::getAttribute<float>(manifest, "pivot-x", 0);
    const float pivotY = Documents::getAttribute<float>(manifest, "pivot-y", 0);
    const uint32_t flowx = marginX + tileWidth;
    const uint32_t flowy = marginY + tileHeight;

    Rect bounds;
    if(type != -1)
        atlas.getOriginalPosition(type, bounds);
    else
        bounds = Rect::parse(manifest);

    const int32_t xCount = Math::round(bounds.width() / flowx);
    const int32_t yCount = Math::round(bounds.height() / flowy);
    const uint32_t bl = static_cast<uint32_t>(bounds.left());
    const uint32_t bt = static_cast<uint32_t>(bounds.top());
    for(int32_t i = 0; i < yCount; i++)
        for(int32_t j = 0; j < xCount; j++)
        {
            uint32_t left = bl + j * flowx;
            uint32_t top = bt + i * flowy;
            atlas.add(++type, left, top, left + tileWidth, top + tileHeight, pivotX, pivotY);
        }
}

sp<Atlas::Importer> AtlasImporterTiles::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<AtlasImporterTiles>::make();
}

}
