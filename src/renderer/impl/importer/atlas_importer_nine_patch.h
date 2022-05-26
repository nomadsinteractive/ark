#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_NINE_PATCH_H_

#include "core/inf/builder.h"
#include "core/inf/storage.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class AtlasImporterNinePatch : public AtlasImporter {
public:
    AtlasImporterNinePatch(document manifest);

    virtual void import(Atlas& atlas, const sp<Readable>& readable) override;

//  [[plugin::builder("nine-patch")]]
    class BUILDER : public Builder<AtlasImporter> {
    public:
        BUILDER(const document& manifest);

        virtual sp<AtlasImporter> build(const Scope& args) override;

    private:
        document _manifest;
    };

private:
    document _manifest;
};

}

#endif
