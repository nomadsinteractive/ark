#include "graphics/impl/storage/tileset_importer_tsx.h"

#include "core/base/named_hash.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/components/render_object.h"
#include "graphics/base/tile.h"
#include "graphics/base/tileset.h"

namespace ark {

void TilesetImporterTsx::import(Tileset& tileset, const sp<Readable>& src)
{
    const document manifest = Documents::loadFromReadable(src);
    for(const document& i : manifest->children("tile"))
    {
        int32_t id = Documents::ensureAttribute<int32_t>(i, "id");
        String type = Documents::getAttribute(i, "class");
        int32_t shapeId = Documents::getAttribute<int32_t>(i, "shape_id", -1);
        const document image = i->getChild("image");
        ASSERT(image);
        const String source = Documents::ensureAttribute(image, "source");
        auto [path, name] = source.rcut('/');
        auto [stem, ext] = name.rcut('.');
        tileset.addTile(sp<Tile>::make(id, std::move(type), shapeId, sp<RenderObject>::make(Strings::eval<int32_t>(stem.value()), nullptr, tileset.tileSize())));
    }
}

sp<TilesetImporter> TilesetImporterTsx::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<TilesetImporterTsx>::make();
}

}
