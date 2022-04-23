#include "graphics/impl/storage/tileset_importer_tsx.h"

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
        int32_t type = Documents::getAttribute<int32_t>(i, "type", 0);
        const document image = i->getChild("image");
        DASSERT(image);
        const String source = Documents::ensureAttribute(image, "source");
        String path, name;
        Strings::rcut(source, path, name, '/');
        String stem, ext;
        Strings::rcut(name, stem, ext, '.');
        tileset.addTile(sp<Tile>::make(id, type, sp<RenderObject>::make(Strings::parse<int32_t>(stem))));
    }
}

sp<TilesetImporter> TilesetImporterTsx::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<TilesetImporterTsx>::make();
}

}
