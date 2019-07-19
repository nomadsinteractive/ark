#ifndef ARK_GRAPHICS_IMPL_IMPORTER_TILE_MAP_IMPORTER_CSV_H_
#define ARK_GRAPHICS_IMPL_IMPORTER_TILE_MAP_IMPORTER_CSV_H_

#include "core/inf/builder.h"
#include "core/inf/importer.h"

#include "graphics/forwarding.h"

namespace ark {

class TileMapImporterCsv : public TileMapImporter {
public:

    virtual void import(TileMap& tilemap, const sp<Readable>& src) override;

//  [[plugin::builder::by-value("csv")]]
    class DICTIONARY : public Builder<TileMapImporter> {
    public:
        DICTIONARY() = default;

        virtual sp<TileMapImporter> build(const sp<Scope>& args) override;
    };

};

}

#endif
