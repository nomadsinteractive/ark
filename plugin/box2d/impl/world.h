#ifndef ARK_PLUGIN_BOX2D_IMPL_WORLD_H_
#define ARK_PLUGIN_BOX2D_IMPL_WORLD_H_

#include <Box2D/Box2D.h>

#include "core/base/bean_factory.h"
#include "core/base/object.h"
#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/collider.h"

#include "box2d/api.h"
#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

class ARK_PLUGIN_BOX2D_API World : public Object, public Runnable, public Collider, Implements<World, Object, Runnable, Collider> {
public:
    World(const b2Vec2& gravity, float ppmX, float ppmY);
    DEFAULT_COPY_AND_ASSIGN(World);

    virtual void run() override;

    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& rotate) override;

    b2World& world() const;

    b2Body* createBody(const b2BodyDef& bodyDef) const;
    b2Body* createBody(Collider::BodyType type, float x, float y, const sp<Size>& size, Shape& shape, float density, float friction) const;

//  [[script::bindings::meta(absorb())]]
//  [[script::bindings::meta(expire())]]
//  [[script::bindings::meta(isExpired())]]

//  [[script::bindings::auto]]
    float toPixelX(float meterX) const;
//  [[script::bindings::auto]]
    float toPixelY(float meterY) const;

//  [[script::bindings::auto]]
    float toMeterX(float pixelX) const;
//  [[script::bindings::auto]]
    float toMeterY(float pixelY) const;

//  [[plugin::resource-loader]]
    class BUILDER_IMPL1 : public Builder<World> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<World> build(const sp<Scope>& args) override;

    private:
        void createBody(Collider::BodyType type, const sp<World>& world, const document& manifest, const sp<Scope>& args);

    private:
        BeanFactory _factory;
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Numeric>> _ppmx, _ppmy;
        sp<Builder<Numeric>> _gravity_x, _gravity_y;
        sp<Builder<Boolean>> _expired;
    };

//  [[plugin::resource-loader("b2World")]]
    class BUILDER_IMPL2 : public Builder<Collider> {
    public:
        BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const sp<Scope>& args) override;

    private:
        BUILDER_IMPL1 _delegate;
    };

private:
    struct BodyManifest {
        BodyManifest();
        BodyManifest(const sp<Shape> shape, float density, float friction);

        sp<Shape> shape;
        float density;
        float friction;
    };

    struct Stub : public Runnable {
        Stub(const b2Vec2& gravity, float ppmX, float ppmY);

        virtual void run() override;

        float _ppm_x;
        float _ppm_y;

        float _time_step;
        int32_t _velocity_iterations;
        int32_t _position_iterations;

        b2World _world;
        std::unordered_map<int32_t, BodyManifest> _body_manifests;
    };

private:
    const sp<Stub> _stub;

    friend class BUILDER_IMPL1;
};

}
}
}

#endif
