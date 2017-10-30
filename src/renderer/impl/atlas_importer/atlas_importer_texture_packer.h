#ifndef ARK_RENDERER_IMPL_ATLAS_IMPORTER_ATLAS_IMPORTER_TEXTURE_PACKER_H_
#define ARK_RENDERER_IMPL_ATLAS_IMPORTER_ATLAS_IMPORTER_TEXTURE_PACKER_H_

#include "core/inf/builder.h"

#include "renderer/base/atlas.h"

namespace ark {

class AtlasImporterTexturePacker : public Atlas::Importer {
public:

    virtual void import(Atlas& atlas, const ResourceLoaderContext& resourceLoaderContext, const document& manifest) override;

//  [[plugin::builder::by-value("texture-packer")]]
    class DICTIONARY : public Builder<Atlas::Importer> {
    public:
        DICTIONARY() = default;

        virtual sp<Atlas::Importer> build(const sp<Scope>& args) override;
    };

};

}

#endif
