#include "renderer/impl/importer/atlas_importer_tiles.h"

#include "core/util/math.h"
#include "core/util/documents.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture.h"

namespace ark {

void AtlasImporterTiles::import(Atlas& atlas, BeanFactory& /*factory*/, const document& manifest)
{
    int32_t type = Documents::getAttribute<int32_t>(manifest, Constants::Attributes::TYPE, -1);
    const uint32_t tileWidth = Documents::ensureAttribute<uint32_t>(manifest, "tile-width");
    const uint32_t tileHeight = Documents::ensureAttribute<uint32_t>(manifest, "tile-height");
    const uint32_t marginX = Documents::getAttribute<uint32_t>(manifest, "margin-x", 0);
    const uint32_t marginY = Documents::getAttribute<uint32_t>(manifest, "margin-y", 0);
    const float pivotX = Documents::getAttribute<float>(manifest, "pivot-x", 0);
    const float pivotY = Documents::getAttribute<float>(manifest, "pivot-y", 0);
    const bool override = Documents::getAttribute<bool>(manifest, "override", false);
    const uint32_t flowx = marginX + tileWidth;
    const uint32_t flowy = marginY + tileHeight;
    const Rect bounds = type != -1 ? atlas.getOriginalPosition(type) : Rect::parse(manifest);
    const uint32_t xCount = static_cast<uint32_t>(Math::round(bounds.width() / flowx));
    const uint32_t yCount = static_cast<uint32_t>(Math::round(bounds.height() / flowy));
    const uint32_t bl = static_cast<uint32_t>(bounds.left());
    const uint32_t bt = static_cast<uint32_t>(bounds.top());

    type = Documents::getAttribute<int32_t>(manifest, "begin", type + 1) - 1;
    for(uint32_t i = 0; i < yCount; i++)
        for(uint32_t j = 0; j < xCount; j++)
        {
            ++type;
            if(!atlas.has(type) || override)
            {
                uint32_t left = bl + j * flowx;
                uint32_t top = bt + i * flowy;
                atlas.add(type, left, top, left + tileWidth, top + tileHeight, Rect(0, 0, 1.0f, 1.0f), V2(tileWidth, tileHeight), V2(pivotX, pivotY));
            }
        }
}

sp<Atlas::Importer> AtlasImporterTiles::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporterTiles>::make();
}

}
