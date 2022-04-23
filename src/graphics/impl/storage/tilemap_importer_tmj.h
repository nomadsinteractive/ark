#ifndef ARK_GRAPHICS_IMPL_STORAGE_TILEMAP_IMPORTER_TMJ_H_
#define ARK_GRAPHICS_IMPL_STORAGE_TILEMAP_IMPORTER_TMJ_H_

#include "core/inf/builder.h"
#include "core/inf/storage.h"

#include "graphics/forwarding.h"

namespace ark {

class TilemapImporterTmj : public Importer<Tilemap> {
public:

    virtual void import(Tilemap& tilemap, const sp<Readable>& src) override;

//  [[plugin::builder::by-value("tmj")]]
    class DICTIONARY : public Builder<Importer<Tilemap>> {
    public:
        DICTIONARY() = default;

        virtual sp<Importer<Tilemap>> build(const Scope& args) override;
    };

};

}

#endif
