#pragma once

#include "plugin/bullet/base/collider_bullet.h"

namespace ark::plugin::bullet {

class ConvexHullRigidBodyImporter : public ColliderBullet::RigidbodyImporter {
public:
    ConvexHullRigidBodyImporter(sp<ModelLoader> modelLoader);

    virtual void import(ColliderBullet& collider, const document& manifest) override;

//  [[plugin::builder("ConvexHull")]]
    class BUILDER : public Builder<ColliderBullet::RigidbodyImporter> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ColliderBullet::RigidbodyImporter> build(const Scope& args) override;

    private:
        SafePtr<Builder<ModelLoader>> _model_loader;
    };

private:
    sp<CollisionShape> makeCollisionShape(const Model& model, btScalar mass);

private:
    sp<ModelLoader> _model_loader;
};

}
