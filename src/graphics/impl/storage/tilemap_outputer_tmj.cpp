#include "graphics/impl/storage/tilemap_outputer_tmj.h"

#include "core/base/json.h"
#include "core/inf/writable.h"
#include "core/util/strings.h"

#include "graphics/base/tile.h"
#include "graphics/base/tilemap.h"
#include "graphics/base/tileset.h"
#include "graphics/base/tilemap_layer.h"

namespace ark {

void TilemapOutputerTmj::output(Tilemap& obj, const sp<Writable>& out)
{
    Json jOut;
    Json jLayers;

    for(const sp<TilemapLayer>& i : obj.layers())
    {
        Json jLayer;
        uint32_t rowCount = i->rowCount();
        uint32_t colCount = i->colCount();
        jLayer.setString("name", i->name());
        jLayer.setInt("width", static_cast<int32_t>(colCount));
        jLayer.setInt("height", static_cast<int32_t>(rowCount));

        std::vector<int32_t> tiles(colCount * rowCount, 0);
        i->foreachTile([&tiles, rowCount, colCount] (uint32_t row, uint32_t col, const sp<Tile>& tile) {
            tiles[(rowCount - 1 - row) * colCount + col] = tile->id() + 1;
            return true;
        });
        jLayer.set("data", sp<Json>::make(sp<IntArray>::make<IntArray::Vector>(std::move(tiles))));

        jLayers.append(jLayer);
    }
    jOut.set("layers", jLayers);
    const String dumped = jOut.dump();
    out->write(dumped.c_str(), static_cast<uint32_t>(dumped.length()), 0);
    out->flush();
}

sp<Outputer<Tilemap>> TilemapOutputerTmj::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<TilemapOutputerTmj>::make();
}

}
