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
    const String dumped = obj.jsonDump().dump();
    out->write(dumped.c_str(), static_cast<uint32_t>(dumped.length()), 0);
}

sp<Outputer<Tilemap>> TilemapOutputerTmj::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<TilemapOutputerTmj>::make();
}

}
