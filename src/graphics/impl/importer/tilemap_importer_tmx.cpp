#include "graphics/impl/importer/tilemap_importer_tmx.h"

#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/render_object.h"
#include "graphics/base/tilemap.h"

namespace ark {

void TilemapImporterTmx::import(Tilemap& tilemap, const sp<Readable>& src)
{
    const document manifest = Documents::loadFromReadable(src);

    tilemap.clear();

    uint32_t rowCount = tilemap.rowCount();
    uint32_t colCount = tilemap.colCount();
    for(const document& i : manifest->children("layer"))
    {
        const document data = i->getChild("data");
        DASSERT(data);
        uint32_t row = 1;
        uint32_t col = 0;
        data->value().split(',', false, [&tilemap, &row, &col, rowCount, colCount](const String& idx) {
            int32_t type = Strings::parse<int32_t>(idx);
            if(type > 0)
                tilemap.setTile(rowCount - row, col, type - 1);
            if(++col == colCount) {
                col = 0;
                ++row;
            }
            return true;
        });
    }
}

sp<TilemapImporter> TilemapImporterTmx::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<TilemapImporterTmx>::make();
}

}
