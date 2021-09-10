#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_GENERIC_XML_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_GENERIC_XML_H_

#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/inf/importer.h"

#include "renderer/forwarding.h"

namespace ark {

class AtlasImporterGenericXML : public AtlasImporter {
public:
    AtlasImporterGenericXML(String src, float px, float py);

    virtual void import(Atlas& atlas, const sp<Readable>& readable) override;

//  [[plugin::builder("generic-xml")]]
    class BUILDER : public Builder<AtlasImporter> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<AtlasImporter> build(const Scope& args) override;

    private:
        sp<Builder<String>> _src;
        float _px;
        float _py;
    };

private:
    String _src;
    float _px;
    float _py;

};

}

#endif
