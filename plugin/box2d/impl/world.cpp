#include "box2d/impl/world.h"

#include "core/util/bean_utils.h"

#include "graphics/base/bounds.h"
#include "graphics/base/rect.h"
#include "graphics/base/vec2.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "box2d/impl/body.h"

namespace ark {
namespace plugin {
namespace box2d {

World::World(const b2Vec2& gravity, float ppmX, float ppmY)
    : _ppm_x(ppmX), _ppm_y(ppmY), _time_step(1.0f / 60.0f), _velocity_iterations(6), _position_iterations(2), _world(gravity)
{
}

void World::run()
{
    _world.Step(_time_step, _velocity_iterations, _position_iterations);
}

sp<RigidBody> World::createBody(Collider::BodyType type, int32_t shape, const sp<VV>& position, const sp<Size>& size, const sp<Transform>& transform)
{
    return nullptr;
}

const b2World& World::world() const
{
    return _world;
}

b2World& World::world()
{
    return _world;
}

b2Body* World::createBody(const b2BodyDef& bodyDef)
{
    return _world.CreateBody(&bodyDef);
}

b2Body* World::createBody(Collider::BodyType type, float x, float y, Shape& shape, float density, float friction)
{
    b2BodyDef bodyDef;
    switch(type)
    {
    case Collider::BODY_TYPE_DYNAMIC:
        bodyDef.type = b2_dynamicBody;
        break;
    case Collider::BODY_TYPE_KINEMATIC:
        bodyDef.type = b2_kinematicBody;
        break;
    case Collider::BODY_TYPE_STATIC:
        bodyDef.type = b2_staticBody;
        break;
    default:
        DFATAL("Illegal collider BodyType: %d", type);
        break;
    }
    bodyDef.position.Set(x, y);
    b2Body* body = createBody(bodyDef);

    DWARN(type != Collider::BODY_TYPE_STATIC || density == 0, "Static body with density %.2f, which usually has no effect", density);
    DWARN(type != Collider::BODY_TYPE_KINEMATIC || density == 0, "Kinematic body with density %.2f, which usually has no effect", density);
    shape.apply(body, density, friction);
    return body;
}

float World::toPixelX(float meter) const
{
	return _ppm_x * meter;
}

float World::toPixelY(float meter) const
{
	return _ppm_y * meter;
}

float World::toMeterX(float pixelX) const
{
    return pixelX / _ppm_x;
}

float World::toMeterY(float pixelY) const
{
    return pixelY / _ppm_y;
}

World::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _expired(factory.getBuilder<Boolean>(manifest, Constants::Attributes::EXPIRED))
{
    BeanUtils::parse<Numeric, Numeric>(factory, manifest, "pixel-per-meter", _ppmx, _ppmy);
    BeanUtils::parse<Numeric, Numeric>(factory, manifest, "gravity", _gravity_x, _gravity_y);
}

sp<World> World::BUILDER_IMPL1::build(const sp<Scope>& args)
{
    b2Vec2 gravity(BeanUtils::toFloat(_gravity_x, args), BeanUtils::toFloat(_gravity_y, args));
    sp<World> world = sp<World>::make(gravity, BeanUtils::toFloat(_ppmx, args), BeanUtils::toFloat(_ppmy, args));
    for(const document& i : _manifest->children("body"))
        createBody(Documents::ensureAttribute<Collider::BodyType>(i, Constants::Attributes::TYPE), world, i, args);
    const sp<Boolean> expired = _expired->build(args);
    if(expired)
        world.absorb(sp<Expired>::make(expired));
    _resource_loader_context->renderController()->addPreUpdateRequest(world, expired ? expired : sp<BooleanByWeakRef<World>>::make(world, 1));
    return world;
}

void World::BUILDER_IMPL1::createBody(Collider::BodyType type, const sp<World>& world, const document& manifest, const sp<Scope>& args)
{
    const String ref = Documents::getAttribute(manifest, Constants::Attributes::ID);
    float density = Documents::getAttribute<float>(manifest, "density", 0);
    float friction = Documents::getAttribute<float>(manifest, "friction", 0.2f);
    const sp<Shape> shape = _factory.ensure<Shape>(manifest, args);
    const sp<VV> pos = _factory.ensure<VV>(manifest, Constants::Attributes::POSITION, args);
    const V v2 = pos->val();
    b2Body* body = world->createBody(type, v2.x(), v2.y(), shape, density, friction);
    if(ref)
        _factory.setReference<Object>(ref, sp<Body>::make(world, body));
}

World::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _delegate(factory, manifest, resourceLoaderContext)
{
}

sp<Object> World::BUILDER_IMPL2::build(const sp<Scope>& args)
{
    return _delegate.build(args);
}

}
}
}
