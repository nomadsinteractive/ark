#ifndef ARK_PLUGIN_BOX2D_IMPL_WORLD_H_
#define ARK_PLUGIN_BOX2D_IMPL_WORLD_H_

#include <Box2D/Box2D.h>

#include "core/base/bean_factory.h"
#include "core/base/object.h"
#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "box2d/api.h"
#include "box2d/impl/body.h"
#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

class ARK_PLUGIN_BOX2D_API World : public Object, public Runnable, Implements<World, Object, Runnable> {
public:
    World(const b2Vec2& gravity, float ppmX, float ppmY);

    virtual void run() override;

    const b2World& world() const;
    b2World& world();

    b2Body* createBody(const b2BodyDef& bodyDef);
    b2Body* createBody(Body::Type type, float x, float y, Shape& shape, float density, float friction);

//  [[script::bindings::auto]]
    float toPixelX(float meterX) const;
//  [[script::bindings::auto]]
    float toPixelY(float meterY) const;

//  [[script::bindings::auto]]
    float toMeterX(float pixelX) const;
//  [[script::bindings::auto]]
    float toMeterY(float pixelY) const;

//  [[plugin::builder]]
    class BUILDER_IMPL1 : public Builder<World> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest);

        virtual sp<World> build(const sp<Scope>& args) override;

    private:
        void createBody(Body::Type type, const sp<World>& world, const document& manifest, const sp<Scope>& args);

    private:
        BeanFactory _parent;
        document _manifest;
        sp<Builder<Numeric>> _ppmx, _ppmy;
        sp<Builder<Numeric>> _gravity_x, _gravity_y;
    };

//  [[plugin::builder("b2World")]]
    class BUILDER_IMPL2 : public Builder<Object> {
    public:
        BUILDER_IMPL2(BeanFactory& parent, const document& doc);

        virtual sp<Object> build(const sp<Scope>& args) override;

    private:
        BUILDER_IMPL1 _delegate;
    };

private:
    float _ppm_x;
    float _ppm_y;

    float _time_step;
    int32_t _velocity_iterations;
    int32_t _position_iterations;

    b2World _world;
};

}
}
}

#endif
