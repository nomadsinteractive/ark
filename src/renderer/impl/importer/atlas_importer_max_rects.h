#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_MAX_RECTS_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_MAX_RECTS_H_

#include "core/inf/builder.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture.h"

namespace ark {

class AtlasImporterMaxRects : public Atlas::Importer {
public:
    AtlasImporterMaxRects(const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void import(Atlas& atlas, BeanFactory& factory, const document& manifest) override;

//  [[plugin::resource-loader("max-rects")]]
    class BUILDER : public Builder<Atlas::Importer> {
    public:
        BUILDER(const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Atlas::Importer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
    };

private:
    struct PackedBitmap {
        String _src;
        int32_t _x;
        int32_t _y;
    };

private:
    sp<ResourceLoaderContext> _resource_loader_context;

};

}

#endif