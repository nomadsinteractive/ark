#include "graphics/impl/storage/tilemap_importer_tmx.h"

#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/tilemap.h"
#include "graphics/base/tileset.h"
#include "graphics/base/tilemap_layer.h"

namespace ark {

void TilemapImporterTmx::import(Tilemap& tilemap, const sp<Readable>& src)
{
    const document manifest = Documents::loadFromReadable(src);

    tilemap.clear();

    for(const document& i : manifest->children("layer"))
    {
        String name = Documents::getAttribute<String>(i, "name", "");
        uint32_t rowCount = Documents::ensureAttribute<uint32_t>(i, "height");
        uint32_t colCount = Documents::ensureAttribute<uint32_t>(i, "width");

        const sp<TilemapLayer> layer = tilemap.makeLayer(std::move(name), rowCount, colCount);
        const document data = i->getChild("data");
        DASSERT(data);
        uint32_t row = 1;
        uint32_t col = 0;
        data->value().split(',', false, [&layer, &row, &col, rowCount, colCount](const String& idx) {
            int32_t type = Strings::parse<int32_t>(idx);
            if(type > 0) {
                layer->setTile(rowCount - row, col, type - 1);
            }
            if(++col == colCount) {
                col = 0;
                ++row;
            }
            return true;
        });
    }
}

sp<Importer<Tilemap> > TilemapImporterTmx::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<TilemapImporterTmx>::make();
}

}
