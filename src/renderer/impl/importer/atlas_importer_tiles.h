#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_TILES_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_TILES_H_

#include "core/inf/builder.h"

#include "renderer/base/atlas.h"

namespace ark {

class AtlasImporterTiles : public Atlas::Importer {
public:
    AtlasImporterTiles(document manifest);

    virtual void import(Atlas& atlas) override;

//  [[plugin::builder("tiles")]]
    class BUILDER : public Builder<Atlas::Importer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Atlas::Importer> build(const Scope& args) override;

    private:
        document _manifest;
    };

private:
    document _manifest;

};

}

#endif
