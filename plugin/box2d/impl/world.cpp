#include "box2d/impl/world.h"

#include "core/util/bean_utils.h"

#include "graphics/base/bounds.h"
#include "graphics/base/rotate.h"
#include "graphics/base/rect.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "box2d/impl/body.h"

#include "box2d/impl/shapes/ball.h"
#include "box2d/impl/shapes/box.h"

namespace ark {
namespace plugin {
namespace box2d {

World::World(const b2Vec2& gravity, float ppmX, float ppmY)
    : _stub(sp<Stub>::make(gravity, ppmX, ppmY))
{
    const BodyManifest box(sp<Box>::make(), 1.0f, 0.2f);
    _stub->_body_manifests[Collider::BODY_SHAPE_AABB] = box;
    _stub->_body_manifests[Collider::BODY_SHAPE_BALL] = BodyManifest(sp<Ball>::make(), 1.0f, 0.2f);
    _stub->_body_manifests[Collider::BODY_SHAPE_BOX] = box;
}

void World::run()
{
    _stub->run();
}

sp<RigidBody> World::createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& rotate)
{
    const auto iter = _stub->_body_manifests.find(shape);
    DCHECK(iter != _stub->_body_manifests.end(), "RigidBody shape-id: %d not found", shape);
    const V pos = position->val();
    const BodyManifest& manifest = iter->second;
    const sp<Body> body = sp<Body>::make(*this, type, pos.x(), pos.y(), size, manifest.shape, manifest.density, manifest.friction);
    if(rotate)
        body->setRotation(rotate->rotation()->val());
    return body;
}

b2World& World::world() const
{
    return _stub->_world;
}

b2Body* World::createBody(const b2BodyDef& bodyDef) const
{
    return _stub->_world.CreateBody(&bodyDef);
}

b2Body* World::createBody(Collider::BodyType type, float x, float y, const sp<Size>& size, Shape& shape, float density, float friction) const
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
    shape.apply(body, size, density, friction);
    return body;
}

float World::toPixelX(float meter) const
{
    return _stub->_ppm_x * meter;
}

float World::toPixelY(float meter) const
{
    return _stub->_ppm_y * meter;
}

float World::toMeterX(float pixelX) const
{
    return pixelX / _stub->_ppm_x;
}

float World::toMeterY(float pixelY) const
{
    return pixelY / _stub->_ppm_y;
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
    const sp<World> world = sp<World>::make(gravity, BeanUtils::toFloat(_ppmx, args), BeanUtils::toFloat(_ppmy, args));
    for(const document& i : _manifest->children("rigid-body"))
    {
        BodyManifest bodyManifest;
        bodyManifest.shape = _factory.ensure<Shape>(i, "shape", args);
        bodyManifest.density = Documents::getAttribute<float>(i, "density", 1.0f);
        bodyManifest.friction = Documents::getAttribute<float>(i, "friction", 0.2f);
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
        world->_stub->_body_manifests[type] = bodyManifest;
    }
    const sp<Boolean> expired = _expired->build(args);
    _resource_loader_context->renderController()->addPreUpdateRequest(world->_stub, expired ? expired : sp<Boolean>::adopt(new BooleanByWeakRef<World::Stub>(world->_stub, 1)));
    return world;
}

World::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _delegate(factory, manifest, resourceLoaderContext)
{
}

sp<Collider> World::BUILDER_IMPL2::build(const sp<Scope>& args)
{
    return _delegate.build(args);
}

World::Stub::Stub(const b2Vec2& gravity, float ppmX, float ppmY)
    : _ppm_x(ppmX), _ppm_y(ppmY), _time_step(1.0f / 60.0f), _velocity_iterations(6), _position_iterations(2), _world(gravity)
{
}

void World::Stub::run()
{
    _world.Step(_time_step, _velocity_iterations, _position_iterations);
}

World::BodyManifest::BodyManifest()
    : density(0), friction(0)
{
}

World::BodyManifest::BodyManifest(const sp<Shape> shape, float density, float friction)
    : shape(shape), density(density), friction(friction)
{
}

}
}
}
