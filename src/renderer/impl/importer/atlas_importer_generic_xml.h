#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_GENERIC_XML_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_GENERIC_XML_H_

#include "core/inf/builder.h"

#include "renderer/base/atlas.h"

namespace ark {

class AtlasImporterGenericXML : public Atlas::Importer {
public:
    AtlasImporterGenericXML(String src, float px, float py);

    virtual void import(Atlas& atlas) override;

//  [[plugin::builder("generic-xml")]]
    class BUILDER : public Builder<Atlas::Importer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Atlas::Importer> build(const Scope& args) override;

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
