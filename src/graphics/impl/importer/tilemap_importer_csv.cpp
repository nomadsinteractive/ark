#include "graphics/impl/importer/tilemap_importer_csv.h"

#include <unordered_map>

#include "core/util/strings.h"

#include "graphics/base/tilemap.h"
#include "graphics/base/render_object.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

void TilemapImporterCsv::import(Tilemap& tilemap, const sp<Readable>& src)
{
    uint32_t rowCount = 0;
    const String content = Strings::loadFromReadable(src);
    std::unordered_map<int32_t, sp<RenderObject>> renderObjects;

    tilemap.clear();

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
//        DCHECK(colCount == tilemap.colCount(), "Insufficient columns, got %d, should be %d", colCount, tilemap.colCount());
        ++rowCount;
    }
//    DCHECK(rowCount == tilemap.rowCount(), "Insufficient rows, got %d, should be %d", rowCount, tilemap.rowCount());
}

sp<TilemapImporter> TilemapImporterCsv::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<TilemapImporterCsv>::make();
}

}
