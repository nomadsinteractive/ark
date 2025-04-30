#pragma once

#include "plugin/bullet/base/collider_bullet.h"

namespace ark::plugin::bullet {

class BvhRigidbodyImporter : public ColliderBullet::RigidbodyImporter {
public:
    BvhRigidbodyImporter(sp<ModelLoader> modelLoader);

    virtual void import(ColliderBullet& collider, const document& manifest) override;

//  [[plugin::builder("Bvh")]]
    class BUILDER : public Builder<ColliderBullet::RigidbodyImporter> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ColliderBullet::RigidbodyImporter> build(const Scope& args) override;

    private:
        SafeBuilder<ModelLoader> _model_loader;
    };

private:
    sp<CollisionShapeRef> makeCollisionShape(const Model& model);

private:
    sp<ModelLoader> _model_loader;
};

}
