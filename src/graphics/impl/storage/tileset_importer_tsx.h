#pragma once

#include "core/inf/builder.h"
#include "core/inf/storage.h"

#include "graphics/forwarding.h"

namespace ark {

class TilesetImporterTsx final : public TilesetImporter {
public:

    void import(Tileset& tileset, const sp<Readable>& src) override;

//  [[plugin::builder::by-value("tsx")]]
    class DICTIONARY final : public Builder<TilesetImporter> {
    public:
        DICTIONARY() = default;

        sp<TilesetImporter> build(const Scope& args) override;
    };

};

}
