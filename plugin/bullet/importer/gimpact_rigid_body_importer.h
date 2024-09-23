#pragma once

#include "plugin/bullet/base/collider_bullet.h"

namespace ark::plugin::bullet {

class GImpactRigidBodyImporter : public ColliderBullet::RigidBodyImporter {
public:
    GImpactRigidBodyImporter(sp<ModelLoader> modelLoader);

    void import(ColliderBullet& collider, const document& manifest) override;

//  [[plugin::builder("GImpact")]]
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
