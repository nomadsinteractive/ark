#ifndef ARK_PLUGIN_BOX2D_IMPL_IMPORTER_RIGID_BODY_IMPORTER_GENERIC_XML_H_
#define ARK_PLUGIN_BOX2D_IMPL_IMPORTER_RIGID_BODY_IMPORTER_GENERIC_XML_H_

#include "box2d/impl/collider_box2d.h"

namespace ark {
namespace plugin {
namespace box2d {

class RigidBodyImporterGenericXML : public ColliderBox2D::RigidBodyImporter {
public:

    virtual void import(ColliderBox2D& world, const sp<Readable>& readable) override;

//  [[plugin::resource-loader("generic-xml")]]
    class BUILDER : public Builder<ColliderBox2D::RigidBodyImporter> {
    public:
        BUILDER() = default;

        virtual sp<ColliderBox2D::RigidBodyImporter> build(const Scope& args) override;

    };

};

}
}
}

#endif
