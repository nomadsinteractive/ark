#ifndef ARK_GRAPHICS_IMPL_IMPORTER_TILEMAP_IMPORTER_CSV_H_
#define ARK_GRAPHICS_IMPL_IMPORTER_TILEMAP_IMPORTER_CSV_H_

#include "core/inf/builder.h"
#include "core/inf/importer.h"

#include "graphics/forwarding.h"

namespace ark {

class TilemapImporterCsv : public TilemapImporter {
public:

    virtual void import(Tilemap& tilemap, const sp<Readable>& src) override;

//  [[plugin::builder::by-value("csv")]]
    class DICTIONARY : public Builder<TilemapImporter> {
    public:
        DICTIONARY() = default;

        virtual sp<TilemapImporter> build(const Scope& args) override;
    };

};

}

#endif
