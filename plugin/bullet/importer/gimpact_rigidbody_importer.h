#pragma once

#include "plugin/bullet/base/collider_bullet.h"

namespace ark::plugin::bullet {

class GImpactRigidbodyImporter : public ColliderBullet::RigidbodyImporter {
public:
    GImpactRigidbodyImporter(sp<ModelLoader> modelLoader);

    void import(ColliderBullet& collider, const document& manifest) override;

//  [[plugin::builder("GImpact")]]
    class BUILDER : public Builder<ColliderBullet::RigidbodyImporter> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ColliderBullet::RigidbodyImporter> build(const Scope& args) override;

    private:
        SafeBuilder<ModelLoader> _model_loader;
    };

private:
    sp<ModelLoader> _model_loader;
};

}
