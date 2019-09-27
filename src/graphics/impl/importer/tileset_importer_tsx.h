#ifndef ARK_GRAPHICS_IMPL_IMPORTER_TILESET_IMPORTER_TSX_H_
#define ARK_GRAPHICS_IMPL_IMPORTER_TILESET_IMPORTER_TSX_H_

#include "core/inf/builder.h"
#include "core/inf/importer.h"

#include "graphics/forwarding.h"

namespace ark {

class TilesetImporterTsx : public TilesetImporter {
public:

    virtual void import(Tileset& tileset, const sp<Readable>& src) override;

//  [[plugin::builder::by-value("tsx")]]
    class DICTIONARY : public Builder<TilesetImporter> {
    public:
        DICTIONARY() = default;

        virtual sp<TilesetImporter> build(const Scope& args) override;
    };

};

}

#endif
