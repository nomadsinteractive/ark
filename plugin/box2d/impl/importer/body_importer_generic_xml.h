#ifndef ARK_PLUGIN_BOX2D_IMPL_IMPORTER_BODY_IMPORTER_GENERIC_XML_H_
#define ARK_PLUGIN_BOX2D_IMPL_IMPORTER_BODY_IMPORTER_GENERIC_XML_H_

#include "box2d/impl/collider_box2d.h"

namespace ark {
namespace plugin {
namespace box2d {

class ImporterGenericXML : public ColliderBox2D::Importer {
public:
    ImporterGenericXML(const document& manifest);

    virtual void import(ColliderBox2D& world) override;

//  [[plugin::resource-loader("generic-xml")]]
    class BUILDER : public Builder<ColliderBox2D::Importer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<ColliderBox2D::Importer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<String>> _src;
    };

private:
    document _manifest;
};

}
}
}

#endif
