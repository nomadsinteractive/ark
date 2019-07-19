#include "graphics/impl/importer/tile_map_importer_csv.h"

#include <unordered_map>

#include "core/util/strings.h"

#include "graphics/base/tile_map.h"
#include "graphics/base/render_object.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

void TileMapImporterCsv::import(TileMap& tilemap, const sp<Readable>& src)
{
    const String content = Strings::loadFromReadable(src);

    tilemap.clear();

    uint32_t rowCount = 0;
    std::unordered_map<int32_t, sp<RenderObject>> renderObjects;

    for(const String& i : content.split('\n'))
    {
        uint32_t colCount = 0;
        i.split(',', false, [&tilemap, &colCount, &renderObjects, rowCount](const String& idx) {
            int32_t type = Strings::parse<int32_t>(idx);
            sp<RenderObject>& ro = renderObjects[type];
            if(!ro)
                ro = sp<RenderObject>::make(type);
            tilemap.setTile(rowCount, colCount++, ro);
            return true;
        });
        DCHECK(colCount == tilemap.colCount(), "Insufficient columns, got %d, should be %d", colCount, tilemap.colCount());
        ++rowCount;
    }
    DCHECK(rowCount == tilemap.rowCount(), "Insufficient rows, got %d, should be %d", rowCount, tilemap.rowCount());
}

sp<TileMapImporter> TileMapImporterCsv::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<TileMapImporterCsv>::make();
}

}
