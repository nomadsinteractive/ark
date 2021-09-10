#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_MAX_RECTS_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_MAX_RECTS_H_

#include "core/inf/builder.h"
#include "core/inf/importer.h"

#include "renderer/forwarding.h"

namespace ark {

class AtlasImporterMaxRects : public AtlasImporter {
public:
    AtlasImporterMaxRects(document manifest, sp<ResourceLoaderContext> resourceLoaderContext);

    virtual void import(Atlas& atlas, const sp<Readable>& readable) override;

//  [[plugin::resource-loader("max-rects")]]
    class BUILDER : public Builder<AtlasImporter> {
    public:
        BUILDER(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<AtlasImporter> build(const Scope& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
    };

private:
    document _manifest;
    sp<ResourceLoaderContext> _resource_loader_context;

};

}

#endif
