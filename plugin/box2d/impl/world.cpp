#include "box2d/impl/world.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/bean_utils.h"
#include "core/util/log.h"

#include "graphics/base/bounds.h"
#include "graphics/base/rotation.h"
#include "graphics/base/rect.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"

#include "box2d/impl/body.h"
#include "box2d/impl/shapes/ball.h"
#include "box2d/impl/shapes/box.h"


namespace ark {
namespace plugin {
namespace box2d {

namespace {

class RigidBodyImpl : public RigidBody {
public:
    RigidBodyImpl(const sp<Stub>& stub)
        : RigidBody(stub) {
    }

    virtual void dispose() override {
        _stub = nullptr;
    }

    static sp<RigidBodyImpl> obtain(ObjectPool& objectPool, const Body::Stub& body) {
        Collider::BodyType bodyType = (body._body->GetType() == b2_staticBody) ?
                                      Collider::BODY_TYPE_STATIC :
                                      (body._body->GetType() == b2_kinematicBody ? Collider::BODY_TYPE_KINEMATIC : Collider::BODY_TYPE_DYNAMIC);
        const b2Vec2& position = body._body->GetPosition();
        float rotation = body._body->GetTransform().q.GetAngle();
        const sp<Vec> p = objectPool.obtain<Vec::Const>(V(position.x, position.y));
        const sp<Rotation> rotate = objectPool.obtain<Rotation>(objectPool.obtain<Numeric::Const>(rotation));
        const sp<RigidBody::Stub> stub = objectPool.obtain<RigidBody::Stub>(body._id, bodyType, p, nullptr, rotate);
        return objectPool.obtain<RigidBodyImpl>(stub);
    }

};

}

World::World(const b2Vec2& gravity, float ppmX, float ppmY)
    : _stub(sp<Stub>::make(gravity, ppmX, ppmY))
{
    const BodyManifest box(sp<Box>::make(), 1.0f, 0.2f);
    _stub->_body_manifests[Collider::BODY_SHAPE_AABB] = box;
    _stub->_body_manifests[Collider::BODY_SHAPE_BALL] = BodyManifest(sp<Ball>::make(), 1.0f, 0.2f);
    _stub->_body_manifests[Collider::BODY_SHAPE_BOX] = box;
    _stub->_world.SetContactListener(&_stub->_contact_listener);
}

void World::run()
{
    _stub->run();
}

sp<RigidBody> World::createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotation>& rotate)
{
    const auto iter = _stub->_body_manifests.find(shape);
    DCHECK(iter != _stub->_body_manifests.end(), "RigidBody shape-id: %d not found", shape);
    const BodyManifest& manifest = iter->second;
    const sp<Body> body = sp<Body>::make(*this, type, position, size, rotate ? rotate->value().cast<Numeric>() : sp<Numeric>::null(), manifest.shape, manifest.density, manifest.friction);
    if(rotate)
        body->setAngle(rotate->radians());

    if(manifest.category || manifest.mask || manifest.group)
    {
        b2Filter filter;
        filter.categoryBits = manifest.category;
        filter.maskBits = manifest.mask;
        filter.groupIndex = manifest.group;
        for(b2Fixture* fixture = body->body()->GetFixtureList(); fixture; fixture = fixture->GetNext())
            fixture->SetFilterData(filter);
    }

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

b2Body* World::createBody(Collider::BodyType type, const V& position, const sp<Size>& size, Shape& shape, float density, float friction) const
{
    b2BodyDef bodyDef;
    switch(type & Collider::BODY_TYPE_MASK)
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
    bodyDef.position.Set(position.x(), position.y());
    b2Body* body = createBody(bodyDef);

    DWARN(type != Collider::BODY_TYPE_STATIC || density == 0, "Static body with density %.2f, which usually has no effect", density);
    DWARN(type != Collider::BODY_TYPE_KINEMATIC || density == 0, "Kinematic body with density %.2f, which usually has no effect", density);
    shape.apply(body, size, density, friction);
    return body;
}

int32_t World::genRigidBodyId() const
{
    return ++_stub->_rigid_body_id_base;
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
    BeanUtils::split<Numeric, Numeric>(factory, manifest, "pixel-per-meter", _ppmx, _ppmy);
    BeanUtils::split<Numeric, Numeric>(factory, manifest, "gravity", _gravity_x, _gravity_y);
}

sp<World> World::BUILDER_IMPL1::build(const sp<Scope>& args)
{
    b2Vec2 gravity(BeanUtils::toFloat(_gravity_x, args), BeanUtils::toFloat(_gravity_y, args));
    const sp<World> world = sp<World>::make(gravity, BeanUtils::toFloat(_ppmx, args), BeanUtils::toFloat(_ppmy, args));
    for(const document& i : _manifest->children("rigid-body"))
    {
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);

        BodyManifest bodyManifest(_factory.ensure<Shape>(i, "shape", args), Documents::getAttribute<float>(i, "density", 1.0f), Documents::getAttribute<float>(i, "friction", 0.2f));
        bodyManifest.category = Documents::getAttribute<uint16_t>(i, "category", 0);
        bodyManifest.mask = Documents::getAttribute<uint16_t>(i, "mask", 0);
        bodyManifest.group = Documents::getAttribute<int16_t>(i, "group", 0);
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
    : _ppm_x(ppmX), _ppm_y(ppmY), _time_step(1.0f / 60.0f), _velocity_iterations(6), _position_iterations(2), _rigid_body_id_base(0), _world(gravity)
{
}

void World::Stub::run()
{
    _world.Step(_time_step, _velocity_iterations, _position_iterations);
}

World::BodyManifest::BodyManifest()
    : density(0), friction(0), category(0), mask(0), group(0)
{
}

World::BodyManifest::BodyManifest(const sp<Shape>& shape, float density, float friction)
    : shape(shape), density(density), friction(friction), category(0), mask(0), group(0)
{
}

void World::ContactListenerImpl::BeginContact(b2Contact* contact)
{
    Body::Stub* body1 = reinterpret_cast<Body::Stub*>(contact->GetFixtureA()->GetBody()->GetUserData());
    Body::Stub* body2 = reinterpret_cast<Body::Stub*>(contact->GetFixtureB()->GetBody()->GetUserData());
    const V normal = V(contact->GetManifold()->localNormal.x, contact->GetManifold()->localNormal.y);
    if(body1 && body2)
    {
        if(body1->_contacts.find(body2->_id) == body1->_contacts.end())
        {
            body1->_contacts.insert(body2->_id);
            body1->_callback->onBeginContact(RigidBodyImpl::obtain(_object_pool, *body2), CollisionManifold(normal));
        }
        if(body2->_contacts.find(body1->_id) == body2->_contacts.end())
        {
            body2->_contacts.insert(body1->_id);
            body2->_callback->onBeginContact(RigidBodyImpl::obtain(_object_pool, *body1), CollisionManifold(-normal));
        }
    }
}

void World::ContactListenerImpl::EndContact(b2Contact* contact)
{
    Body::Stub* body1 = reinterpret_cast<Body::Stub*>(contact->GetFixtureA()->GetBody()->GetUserData());
    Body::Stub* body2 = reinterpret_cast<Body::Stub*>(contact->GetFixtureB()->GetBody()->GetUserData());
    if(body1 && body2)
    {
        const auto it1 = body1->_contacts.find(body2->_id);
        if(it1 != body1->_contacts.end())
        {
            body1->_contacts.erase(it1);
            body1->_callback->onEndContact(RigidBodyImpl::obtain(_object_pool, *body2));
        }
        const auto it2 = body2->_contacts.find(body1->_id);
        if(it2 != body2->_contacts.end())
        {
            body2->_contacts.erase(it2);
            body2->_callback->onEndContact(RigidBodyImpl::obtain(_object_pool, *body1));
        }
    }
}

}
}
}
