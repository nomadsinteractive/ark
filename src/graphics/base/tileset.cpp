#include "graphics/base/tileset.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/asset.h"
#include "core/inf/storage.h"
#include "core/util/documents.h"

#include "graphics/base/render_object.h"
#include "graphics/base/tile.h"

namespace ark {

Tileset::Tileset(uint32_t tileWidth, uint32_t tileHeight, const sp<TilesetImporter>& importer)
    : _tile_width(tileWidth), _tile_height(tileHeight), _importer(importer)
{
}

uint32_t Tileset::tileWidth() const
{
    return _tile_width;
}

uint32_t Tileset::tileHeight() const
{
    return _tile_height;
}

void Tileset::addTile(sp<Tile> t)
{
    sp<Tile>& tile = _tiles[t->id()];
    DWARN(!tile, "Overriding existing tile: %d", t->id());
    tile = std::move(t);
}

const sp<Tile>& Tileset::getTile(int32_t id) const
{
    const auto iter = _tiles.find(id);
    return iter != _tiles.end() ? iter->second : sp<Tile>::null();
}

void Tileset::load(const sp<Readable>& readable)
{
    DASSERT(_importer);
    _importer->import(*this, readable);
}

void Tileset::load(const String& src)
{
    load(Ark::instance().openAsset(src));
}

Tileset::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _tile_width(Documents::ensureAttribute<uint32_t>(manifest, "tile-width")), _tile_height(Documents::ensureAttribute<uint32_t>(manifest, "tile-height")),
      _importer(factory.getBuilder<TilesetImporter>(manifest, "importer"))
{
}

sp<Tileset> Tileset::BUILDER::build(const Scope& args)
{
    return sp<Tileset>::make(_tile_width, _tile_height, _importer->build(args));
}

}
