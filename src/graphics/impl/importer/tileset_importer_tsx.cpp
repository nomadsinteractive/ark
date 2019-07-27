#include "graphics/impl/importer/tileset_importer_tsx.h"

#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/render_object.h"
#include "graphics/base/tileset.h"

namespace ark {

void TilesetImporterTsx::import(Tileset& tileset, const sp<Readable>& src)
{
    const document manifest = Documents::loadFromReadable(src);
    for(const document& i : manifest->children("tile"))
    {
        int32_t id = Documents::ensureAttribute<int32_t>(i, "id");
        const document image = i->getChild("image");
        DASSERT(image);
        const String source = Documents::ensureAttribute(image, "source");
        String path, name;
        Strings::rcut(source, path, name, '/');
        String type, ext;
        Strings::rcut(name, type, ext, '.');
        tileset.addTile(id, sp<RenderObject>::make(Strings::parse<int32_t>(type)));
    }
}

sp<TilesetImporter> TilesetImporterTsx::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<TilesetImporterTsx>::make();
}

}
