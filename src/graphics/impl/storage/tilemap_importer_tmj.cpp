#include "graphics/impl/storage/tilemap_importer_tmj.h"

#include "core/base/json.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/base/tilemap.h"
#include "graphics/base/tileset.h"
#include "graphics/base/tilemap_layer.h"

namespace ark {

void TilemapImporterTmj::import(Tilemap& tilemap, const sp<Readable>& src)
{
    Json json;
    json.load(Strings::loadFromReadable(src));

    tilemap.clear();

    const sp<Json> layers = json.get("layers");
    DASSERT(layers->isArray());
    for(int32_t i = 0; i < layers->size(); ++i)
    {
        const sp<Json> layer = layers->at(i);
        DASSERT(layer);
        String name = layer->getString("name");
        uint32_t rowCount = layer->getInt("height");
        uint32_t colCount = layer->getInt("width");

        const sp<TilemapLayer> tilemapLayer = tilemap.makeLayer(std::move(name), rowCount, colCount);
        const sp<Json> data = layer->get("data");
        DASSERT(data);
        DASSERT(data->isArray());
        uint32_t row = 1;
        uint32_t col = 0;
        data->foreach([&tilemapLayer, &row, &col, rowCount, colCount](const Json& idx) {
            int32_t type = idx.toInt();
            if(type > 0) {
                tilemapLayer->setTile(rowCount - row, col, type - 1);
            }
            if(++col == colCount) {
                col = 0;
                ++row;
            }
            return true;
        });
    }
}

sp<Importer<Tilemap>> TilemapImporterTmj::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<TilemapImporterTmj>::make();
}

}
