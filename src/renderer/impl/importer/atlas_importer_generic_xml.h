#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_GENERIC_XML_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_GENERIC_XML_H_

#include "core/inf/builder.h"

#include "renderer/base/atlas.h"

namespace ark {

class AtlasImporterGenericXML : public Atlas::Importer {
public:

    virtual void import(Atlas& atlas, const document& manifest) override;

//  [[plugin::builder("generic-xml")]]
    class BUILDER : public Builder<Atlas::Importer> {
    public:
        BUILDER() = default;

        virtual sp<Atlas::Importer> build(const Scope& args) override;
    };

};

}

#endif
