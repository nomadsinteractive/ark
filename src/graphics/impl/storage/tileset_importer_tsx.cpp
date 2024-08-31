#include "graphics/impl/storage/tileset_importer_tsx.h"

#include "core/base/named_type.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/render_object.h"
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
        DASSERT(image);
//        uint32_t width = Documents::getAttribute<uint32_t>(image, "width", 0);
//        uint32_t height = Documents::getAttribute<uint32_t>(image, "height", 0);
        const String source = Documents::ensureAttribute(image, "source");
        String path, name;
        Strings::rcut(source, path, name, '/');
        String stem, ext;
        Strings::rcut(name, stem, ext, '.');
        tileset.addTile(sp<Tile>::make(id, std::move(type), shapeId, sp<RenderObject>::make(Strings::eval<int32_t>(stem), nullptr, tileset.tileSize())));
    }
}

sp<TilesetImporter> TilesetImporterTsx::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<TilesetImporterTsx>::make();
}

}
