#include "graphics/impl/storage/tilemap_importer_tmj.h"

#include "core/base/json.h"
#include "core/util/strings.h"

#include "graphics/base/tilemap.h"

namespace ark {

void TilemapImporterTmj::import(Tilemap& tilemap, const sp<Readable>& src)
{
    Json json;
    json.load(Strings::loadFromReadable(src));
    tilemap.jsonLoad(json);
}

sp<Importer<Tilemap>> TilemapImporterTmj::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<Importer<Tilemap>>::make<TilemapImporterTmj>();
}

}
