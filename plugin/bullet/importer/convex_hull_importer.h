#ifndef ARK_PLUGIN_BULLET_IMPORTER_CONVEX_HULL_IMPORTER_H_
#define ARK_PLUGIN_BULLET_IMPORTER_CONVEX_HULL_IMPORTER_H_

#include "plugin/bullet/base/collider_bullet.h"

namespace ark {
namespace plugin {
namespace bullet {

class ConvexHullRigidBodyImporter : public ColliderBullet::RigidBodyImporter {
public:
    ConvexHullRigidBodyImporter(sp<ModelLoader> modelLoader);

    virtual void import(ColliderBullet& collider, const document& manifest) override;

//  [[plugin::builder("ConvexHull")]]
    class BUILDER : public Builder<ColliderBullet::RigidBodyImporter> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ColliderBullet::RigidBodyImporter> build(const Scope& args) override;

    private:
        SafePtr<Builder<ModelLoader>> _model_loader;
    };

private:
    sp<CollisionShape> makeCollisionShape(const Model& model, btScalar mass);

private:
    sp<ModelLoader> _model_loader;
};

}
}
}

#endif
