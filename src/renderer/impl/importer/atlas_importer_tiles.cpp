#include "renderer/impl/importer/atlas_importer_tiles.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/math.h"
#include "core/util/documents.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture.h"

namespace ark {

AtlasImporterTiles::AtlasImporterTiles(document manifest, int32_t type, uint32_t tileWidth, uint32_t tileHeight, float pivotX, float pivotY)
    : _manifest(std::move(manifest)), _type(type), _tile_width(tileWidth), _tile_height(tileHeight), _pivot(pivotX, pivotY)
{
}

void AtlasImporterTiles::import(Atlas& atlas, const sp<Readable>& /*readable*/)
{
    const uint32_t marginX = Documents::getAttribute<uint32_t>(_manifest, "margin-x", 0);
    const uint32_t marginY = Documents::getAttribute<uint32_t>(_manifest, "margin-y", 0);
    const bool override = Documents::getAttribute<bool>(_manifest, "override", false);
    const uint32_t flowx = marginX + _tile_width;
    const uint32_t flowy = marginY + _tile_height;
    const Rect bounds = _type != -1 ? atlas.getItemBounds(_type) : Rect::parse(_manifest);
    const uint32_t xCount = static_cast<uint32_t>(std::lround(bounds.width() / flowx));
    const uint32_t yCount = static_cast<uint32_t>(std::lround(bounds.height() / flowy));
    const uint32_t bl = static_cast<uint32_t>(bounds.left());
    const uint32_t bt = static_cast<uint32_t>(bounds.top());

    int32_t typeBase = Documents::getAttribute<int32_t>(_manifest, "begin", _type + 1) - 1;
    const V2 tileSize(static_cast<float>(_tile_width), static_cast<float>(_tile_height));
    for(uint32_t i = 0; i < yCount; i++)
        for(uint32_t j = 0; j < xCount; j++)
        {
            ++typeBase;
            if(!atlas.has(typeBase) || override)
            {
                uint32_t left = bl + j * flowx;
                uint32_t top = bt + i * flowy;
                atlas.add(typeBase, left, top, left + _tile_width, top + _tile_height, Rect(0, 0, 1.0f, 1.0f), tileSize, _pivot);
            }
        }
}

AtlasImporterTiles::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _manifest(manifest), _type(factory.getBuilder<Integer>(_manifest, Constants::Attributes::TYPE)), _tile_width(factory.ensureBuilder<Integer>(_manifest, "tile-width")),
      _tile_height(factory.ensureBuilder<Integer>(_manifest, "tile-height")), _pivot_x(factory.getBuilder<Numeric>(_manifest, "pivot-x")), _pivot_y(factory.getBuilder<Numeric>(_manifest, "pivot-y"))
{
}

sp<AtlasImporter> AtlasImporterTiles::BUILDER::build(const Scope& args)
{
    return sp<AtlasImporterTiles>::make(_manifest, _type ? _type->build(args)->val() : -1, _tile_width->build(args)->val(), _tile_height->build(args)->val(),
                                        _pivot_x ? _pivot_x->build(args)->val() : 0, _pivot_y ? _pivot_y->build(args)->val() : 0);
}

}
