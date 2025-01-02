#include "graphics/base/tileset.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/asset.h"
#include "core/inf/storage.h"
#include "core/util/documents.h"

#include "graphics/components/render_object.h"
#include "graphics/base/tile.h"

namespace ark {

Tileset::Tileset(sp<Size> tileSize, sp<TilesetImporter> importer)
    : _tile_size(std::move(tileSize)), _importer(importer)
{
}

const std::unordered_map<int32_t, sp<Tile>>& Tileset::tiles() const
{
    return _tiles;
}

const sp<Size>& Tileset::tileSize() const
{
    return _tile_size;
}

float Tileset::tileWidth() const
{
    return _tile_size->widthAsFloat();
}

float Tileset::tileHeight() const
{
    return _tile_size->heightAsFloat();
}

void Tileset::addTile(sp<Tile> t)
{
    sp<Tile>& tile = _tiles[t->id()];
    DCHECK_WARN(!tile, "Overriding existing tile: %d", t->id());
    tile = std::move(t);
}

sp<Tile> Tileset::getTile(int32_t id) const
{
    const auto iter = _tiles.find(id);
    return iter != _tiles.end() ? iter->second : nullptr;
}

void Tileset::load(const sp<Readable>& src)
{
    DASSERT(_importer);
    _importer->import(*this, src);
}

void Tileset::load(const String& src)
{
    load(Ark::instance().openAsset(src));
}

Tileset::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _tile_width(Documents::ensureAttribute<float>(manifest, "tile-width")), _tile_height(Documents::ensureAttribute<float>(manifest, "tile-height")),
      _importer(factory.getBuilder<TilesetImporter>(manifest, "importer"))
{
}

sp<Tileset> Tileset::BUILDER::build(const Scope& args)
{
    return sp<Tileset>::make(sp<Size>::make(_tile_width, _tile_height), _importer->build(args));
}

}
