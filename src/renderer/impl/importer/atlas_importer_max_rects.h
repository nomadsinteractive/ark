#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_MAX_RECTS_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_MAX_RECTS_H_

#include "core/inf/builder.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture.h"

namespace ark {

class AtlasImporterMaxRects : public Atlas::Importer {
public:
    AtlasImporterMaxRects(document manifest, sp<ResourceLoaderContext> resourceLoaderContext);

    virtual void import(Atlas& atlas) override;

//  [[plugin::resource-loader("max-rects")]]
    class BUILDER : public Builder<Atlas::Importer> {
    public:
        BUILDER(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Atlas::Importer> build(const Scope& args) override;

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
