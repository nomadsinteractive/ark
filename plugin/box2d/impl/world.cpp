#include "box2d/impl/world.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/bean_utils.h"
#include "core/util/boolean_util.h"
#include "core/util/log.h"

#include "graphics/base/bounds.h"
#include "graphics/base/rotate.h"
#include "graphics/base/rect.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"

#include "box2d/impl/body.h"
#include "box2d/impl/joint.h"
#include "box2d/impl/shapes/ball.h"
#include "box2d/impl/shapes/box.h"

namespace ark {
namespace plugin {
namespace box2d {

World::World(const b2Vec2& gravity, float ppmX, float ppmY)
    : _stub(sp<Stub>::make(gravity, ppmX, ppmY))
{
    const BodyCreateInfo box(sp<Box>::make(), 1.0f, 0.2f);
    _stub->_body_manifests[Collider::BODY_SHAPE_AABB] = box;
    _stub->_body_manifests[Collider::BODY_SHAPE_BALL] = BodyCreateInfo(sp<Ball>::make(), 1.0f, 0.2f);
    _stub->_body_manifests[Collider::BODY_SHAPE_BOX] = box;
    _stub->_world.SetContactListener(&_stub->_contact_listener);
    _stub->_world.SetDestructionListener(&_stub->_destruction_listener);
}

void World::run()
{
    _stub->run();
}

sp<RigidBody> World::createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& rotate)
{
    const auto iter = _stub->_body_manifests.find(shape);
    DCHECK(iter != _stub->_body_manifests.end(), "RigidBody shape-id: %d not found", shape);
    const BodyCreateInfo& manifest = iter->second;
    const sp<Body> body = sp<Body>::make(*this, type, position, size, rotate ? rotate->value().cast<Numeric>() : sp<Numeric>::null(), manifest);
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

b2Body* World::createBody(Collider::BodyType type, const V& position, const sp<Size>& size, const BodyCreateInfo& createInfo) const
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

    createInfo.shape->apply(body, size, createInfo);
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

void World::track(const sp<Joint>& joint) const
{
    _stub->_destruction_listener.track(joint);
}

void World::setBodyManifest(int32_t id, const BodyCreateInfo& bodyManifest)
{
    DWARN(_stub->_body_manifests.find(id) == _stub->_body_manifests.end(), "Overriding existing body: %d", id);
    _stub->_body_manifests[id] = bodyManifest;
}

World::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _expired(factory.getBuilder<Boolean>(manifest, Constants::Attributes::EXPIRED))
{
    BeanUtils::split<Numeric, Numeric>(factory, manifest, "pixel-per-meter", _ppmx, _ppmy);
    BeanUtils::split<Numeric, Numeric>(factory, manifest, "gravity", _gravity_x, _gravity_y);

    for(const document& i : _manifest->children("import"))
        _importers.push_back(_factory.ensureBuilder<Importer>(i));
}

sp<World> World::BUILDER_IMPL1::build(const sp<Scope>& args)
{
    b2Vec2 gravity(BeanUtils::toFloat(_gravity_x, args), BeanUtils::toFloat(_gravity_y, args));
    const sp<World> world = sp<World>::make(gravity, BeanUtils::toFloat(_ppmx, args), BeanUtils::toFloat(_ppmy, args));
    for(const document& i : _manifest->children("rigid-body"))
    {
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);

        BodyCreateInfo bodyCreateInfo(_factory.ensure<Shape>(i, "shape", args), Documents::getAttribute<float>(i, "density", 1.0f),
                                      Documents::getAttribute<float>(i, "friction", 0.2f), Documents::getAttribute<bool>(i, "is-sensor", false));
        bodyCreateInfo.category = Documents::getAttribute<uint16_t>(i, "category", 0);
        bodyCreateInfo.mask = Documents::getAttribute<uint16_t>(i, "mask", 0);
        bodyCreateInfo.group = Documents::getAttribute<int16_t>(i, "group", 0);
        world->_stub->_body_manifests[type] = bodyCreateInfo;
    }

    for(const sp<Builder<Importer>>& i : _importers)
    {
        const sp<Importer> importer = i->build(args);
        importer->import(world);
    }

    const sp<Boolean> expired = _expired->build(args);
    _resource_loader_context->renderController()->addPreUpdateRequest(world->_stub, expired ? expired : BooleanUtil::__or__(_resource_loader_context->disposed(), sp<Boolean>::adopt(new BooleanByWeakRef<World::Stub>(world->_stub, 1))));
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

void World::ContactListenerImpl::BeginContact(b2Contact* contact)
{
    Body::Shadow* s1 = reinterpret_cast<Body::Shadow*>(contact->GetFixtureA()->GetBody()->GetUserData());
    Body::Shadow* s2 = reinterpret_cast<Body::Shadow*>(contact->GetFixtureB()->GetBody()->GetUserData());
    const V normal = V(contact->GetManifold()->localNormal.x, contact->GetManifold()->localNormal.y);
    if(s1 && s2)
    {
        const sp<Body::Stub> body1 = s1->_body.ensure();
        const sp<Body::Stub> body2 = s2->_body.ensure();
        if(body1->_contacts.find(body2->_id) == body1->_contacts.end())
        {
            body1->_contacts.insert(body2->_id);
            if(body1->_callback->hasCallback())
                body1->_callback->onBeginContact(Body::obtain(s2, _object_pool), CollisionManifold(normal));
        }
        if(body2->_contacts.find(body1->_id) == body2->_contacts.end())
        {
            body2->_contacts.insert(body1->_id);
            if(body2->_callback->hasCallback())
                body2->_callback->onBeginContact(Body::obtain(s1, _object_pool), CollisionManifold(-normal));
        }
    }
}

void World::ContactListenerImpl::EndContact(b2Contact* contact)
{
    Body::Shadow* s1 = reinterpret_cast<Body::Shadow*>(contact->GetFixtureA()->GetBody()->GetUserData());
    Body::Shadow* s2 = reinterpret_cast<Body::Shadow*>(contact->GetFixtureB()->GetBody()->GetUserData());
    if(s1 && s2)
    {
        const sp<Body::Stub> body1 = s1->_body.ensure();
        const sp<Body::Stub> body2 = s2->_body.ensure();
        const auto it1 = body1->_contacts.find(body2->_id);
        if(it1 != body1->_contacts.end())
        {
            body1->_contacts.erase(it1);
            if(body1->_callback->hasCallback())
                body1->_callback->onEndContact(Body::obtain(s2, _object_pool));
        }
        const auto it2 = body2->_contacts.find(body1->_id);
        if(it2 != body2->_contacts.end())
        {
            body2->_contacts.erase(it2);
            if(body2->_callback->hasCallback())
                body2->_callback->onEndContact(Body::obtain(s1, _object_pool));
        }
    }
}

void World::DestructionListenerImpl::SayGoodbye(b2Joint* joint)
{
    auto iter = _joints.find(joint);
    if(iter != _joints.end())
    {
        const sp<Joint> obj = iter->second.lock();
        if(obj)
            obj->release();

        _joints.erase(iter);
    }
}

void World::DestructionListenerImpl::SayGoodbye(b2Fixture* fixture)
{
}

void World::DestructionListenerImpl::track(const sp<Joint>& joint)
{
    _joints[joint->object()] = joint;
}

void World::DestructionListenerImpl::untrack(const sp<Joint>& joint)
{
    auto iter = _joints.find(joint->object());
    if(iter != _joints.end())
        _joints.erase(iter);
}

}
}
}
