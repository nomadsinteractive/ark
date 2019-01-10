#ifndef ARK_PLUGIN_BOX2D_IMPL_IMPORTER_BODY_IMPORTER_GENERIC_XML_H_
#define ARK_PLUGIN_BOX2D_IMPL_IMPORTER_BODY_IMPORTER_GENERIC_XML_H_

#include "plugin/box2d/impl/world.h"

namespace ark {
namespace plugin {
namespace box2d {

class ImporterGenericXML : public World::Importer {
public:
    ImporterGenericXML(const document& manifest);

    virtual void import(World& world) override;

//  [[plugin::resource-loader("generic-xml")]]
    class BUILDER : public Builder<World::Importer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<World::Importer> build(const sp<Scope>& args) override;

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
