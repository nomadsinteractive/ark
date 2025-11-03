#pragma once

#include "core/inf/builder.h"
#include "core/inf/storage.h"

#include "graphics/base/v2.h"

#include "renderer/forwarding.h"

namespace ark {

class AtlasImporterTiles final : public AtlasImporter {
public:
    AtlasImporterTiles(document manifest, int32_t type, uint32_t tileWidth, uint32_t tileHeight, float pivotX, float pivotY);

    void import(Atlas& atlas, const sp<Readable>& readable) override;

//  [[plugin::builder("tiles")]]
    class BUILDER final : public Builder<AtlasImporter> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<AtlasImporter> build(const Scope& args) override;

    private:
        document _manifest;
        sp<Builder<Integer>> _type;
        sp<Builder<Integer>> _tile_width, _tile_height;
        sp<Builder<Numeric>> _pivot_x, _pivot_y;
    };

private:
    document _manifest;

    int32_t _type;
    uint32_t _tile_width, _tile_height;
    V2 _pivot;
};

}
