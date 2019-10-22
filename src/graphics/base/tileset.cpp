#include "graphics/base/tileset.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/asset.h"
#include "core/inf/importer.h"
#include "core/util/documents.h"

#include "graphics/base/render_object.h"

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

void Tileset::addTile(int32_t id, const sp<RenderObject>& t)
{
    sp<RenderObject>& tile = _tiles[id];
    DWARN(!tile, "Overriding existing tile: %d", id);
    tile = t;
}

const sp<RenderObject>& Tileset::getTile(int32_t id) const
{
    const auto iter = _tiles.find(id);
    return iter != _tiles.end() ? iter->second : sp<RenderObject>::null();
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
