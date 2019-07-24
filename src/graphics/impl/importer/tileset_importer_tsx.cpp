#include "graphics/impl/importer/tileset_importer_tsx.h"

#include "core/util/strings.h"

#include "graphics/base/tileset.h"
#include "graphics/base/render_object.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

void TilesetImporterTsx::import(Tileset& tileset, const sp<Readable>& src)
{

}

sp<TilesetImporter> TilesetImporterTsx::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<TilesetImporterTsx>::make();
}

}
