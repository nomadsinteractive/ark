#include "box2d/impl/collider_box2d.h"

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/boolean_type.h"

#include "graphics/base/rect.h"
#include "graphics/base/v2.h"
#include "graphics/components/rotation.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_manifold.h"
#include "app/base/raycast_manifold.h"
#include "app/inf/collision_callback.h"
#include "app/components/shape.h"

#include "box2d/impl/rigidbody_box2d.h"
#include "box2d/impl/joint.h"
#include "box2d/impl/shapes/ball.h"
#include "box2d/impl/shapes/box.h"
#include "core/types/ref.h"
#include "graphics/base/rotation_axis_theta.h"

namespace ark::plugin::box2d {

ColliderBox2D::ColliderBox2D(const b2Vec2& gravity, const V2& pixelPerMeter)
    : _stub(sp<Stub>::make(gravity, pixelPerMeter))
{
    const BodyCreateInfo box(sp<Box>::make(), 1.0f, 0.2f);
    _stub->_body_manifests[ark::Shape::TYPE_AABB] = box;
    _stub->_body_manifests[ark::Shape::TYPE_BALL] = BodyCreateInfo(sp<Ball>::make(), 1.0f, 0.2f);
    _stub->_body_manifests[ark::Shape::TYPE_BOX] = box;
    _stub->_world.SetContactListener(&_stub->_contact_listener);
    _stub->_world.SetDestructionListener(&_stub->_destruction_listener);
}

void ColliderBox2D::run()
{
    _stub->run();
}

Rigidbody::Impl ColliderBox2D::createBody(Rigidbody::BodyType type, sp<ark::Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
{
    const auto iter = _stub->_body_manifests.find(shape->type().hash());
    CHECK(iter != _stub->_body_manifests.end(), "RigidBody shape-type: %ud not found", shape->type().hash());
    const BodyCreateInfo& manifest = iter->second;
    const sp<Rotation> rot = rotation.asInstance<Rotation>();
    const sp<RotationAxisTheta> axisTheta = rot ? rot.asInstance<RotationAxisTheta>() : sp<RotationAxisTheta>();
    const sp<RigidbodyBox2D> body = sp<RigidbodyBox2D>::make(*this, type, position, shape->size().val(), axisTheta ? axisTheta->theta() : nullptr, std::move(collisionFilter), manifest);
    if(axisTheta)
        body->setAngle(axisTheta->theta()->val());
    CHECK(!discarded, "Unimplemented");

    if(manifest.category || manifest.mask || manifest.group)
    {
        b2Filter filter;
        filter.categoryBits = manifest.category;
        filter.maskBits = manifest.mask;
        filter.groupIndex = manifest.group;
        for(b2Fixture* fixture = body->body()->GetFixtureList(); fixture; fixture = fixture->GetNext())
            fixture->SetFilterData(filter);
    }

    return {body->rigidbodyStub(), nullptr, body};
}

sp<ark::Shape> ColliderBox2D::createShape(const NamedHash& type, sp<Vec3> size, sp<Vec3> origin)
{
    return sp<ark::Shape>::make(type, std::move(size), std::move(origin));
}

Vector<RayCastManifold> ColliderBox2D::rayCast(V3 from, V3 to, const sp<CollisionFilter>& /*collisionFilter*/)
{
    DFATAL("Unimplemented");
    return {};
}

b2World& ColliderBox2D::world() const
{
    return _stub->_world;
}

b2Body* ColliderBox2D::createBody(const b2BodyDef& bodyDef) const
{
    return _stub->_world.CreateBody(&bodyDef);
}

b2Body* ColliderBox2D::createBody(Rigidbody::BodyType type, const V3& position, const V3& size, const BodyCreateInfo& createInfo) const
{
    b2BodyDef bodyDef;
    switch(type & Rigidbody::BODY_TYPE_RIGID)
    {
    case Rigidbody::BODY_TYPE_DYNAMIC:
        bodyDef.type = b2_dynamicBody;
        break;
    case Rigidbody::BODY_TYPE_KINEMATIC:
        bodyDef.type = b2_kinematicBody;
        break;
    case Rigidbody::BODY_TYPE_STATIC:
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

float ColliderBox2D::toPixelX(float meter) const
{
    return _stub->_ppm_x * meter;
}

float ColliderBox2D::toPixelY(float meter) const
{
    return _stub->_ppm_y * meter;
}

float ColliderBox2D::toMeterX(float pixelX) const
{
    return pixelX / _stub->_ppm_x;
}

float ColliderBox2D::toMeterY(float pixelY) const
{
    return pixelY / _stub->_ppm_y;
}

int32_t ColliderBox2D::bodyCount() const
{
    return _stub->_world.GetBodyCount();
}

void ColliderBox2D::track(const sp<Joint::Stub>& joint) const
{
    _stub->_destruction_listener.track(joint);
}

void ColliderBox2D::setBodyManifest(int32_t id, const BodyCreateInfo& bodyManifest)
{
    DCHECK_WARN(_stub->_body_manifests.find(id) == _stub->_body_manifests.end(), "Overriding existing body: %d", id);
    _stub->_body_manifests[id] = bodyManifest;
}

ColliderBox2D::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _ppm(Documents::ensureAttribute<V2>(manifest, "pixel-per-meter")),
      _gravity(Documents::getAttribute<V2>(manifest, "gravity", {0, -9.8f})), _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED))
{
    for(const document& i : _manifest->children("import"))
        _importers.push_back({_factory.ensureBuilder<RigidBodyImporter>(i), Documents::ensureAttribute(i, constants::SRC)});
}

sp<ColliderBox2D> ColliderBox2D::BUILDER_IMPL1::build(const Scope& args)
{
    b2Vec2 gravity(_gravity.x(), _gravity.y());
    const sp<ColliderBox2D> world = sp<ColliderBox2D>::make(gravity, _ppm);
    for(const document& i : _manifest->children("rigid-body"))
    {
        const int32_t type = Documents::ensureAttribute<int32_t>(i, constants::TYPE);
        BodyCreateInfo bodyCreateInfo(_factory.ensure<Shape>(i, "shape", args), Documents::getAttribute<float>(i, "density", 1.0f),
                                      Documents::getAttribute<float>(i, "friction", 0.2f), Documents::getAttribute<bool>(i, "is-sensor", false));
        bodyCreateInfo.category = Documents::getAttribute<uint16_t>(i, "category", 0);
        bodyCreateInfo.mask = Documents::getAttribute<uint16_t>(i, "mask", 0);
        bodyCreateInfo.group = Documents::getAttribute<int16_t>(i, "group", 0);
        world->_stub->_body_manifests[type] = bodyCreateInfo;
    }

    const Ark& ark = Ark::instance();
    for(const auto& [k, v] : _importers)
    {
        const sp<RigidBodyImporter> importer = k->build(args);
        importer->import(*world, ark.openAsset(v));
    }

    const sp<Boolean> discarded = _discarded.build(args);
    _resource_loader_context->renderController()->addPreComposeRunnable(world->_stub, discarded ? discarded : BooleanType::__or__(_resource_loader_context->discarded(), sp<Boolean>::make<BooleanByWeakRef<ColliderBox2D::Stub>>(world->_stub, 1)));
    return world;
}

ColliderBox2D::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _delegate(factory, manifest, resourceLoaderContext)
{
}

sp<Collider> ColliderBox2D::BUILDER_IMPL2::build(const Scope& args)
{
    return _delegate.build(args);
}

ColliderBox2D::Stub::Stub(const b2Vec2& gravity, const V2& pixelPerMeter)
    : _ppm_x(pixelPerMeter.x()), _ppm_y(pixelPerMeter.y()), _time_step(1.0f / 60.0f), _velocity_iterations(6), _position_iterations(2), _world(gravity)
{
}

void ColliderBox2D::Stub::run()
{
    _world.Step(_time_step, _velocity_iterations, _position_iterations);
}

void ColliderBox2D::ContactListenerImpl::BeginContact(b2Contact* contact)
{
    RigidbodyBox2D* s1 = reinterpret_cast<RigidbodyBox2D*>(contact->GetFixtureA()->GetBody()->GetUserData().pointer);
    RigidbodyBox2D* s2 = reinterpret_cast<RigidbodyBox2D*>(contact->GetFixtureB()->GetBody()->GetUserData().pointer);
    if(s1 && s2)
    {
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold);
        const V3 normal = V3(worldManifold.normal.x, worldManifold.normal.y, 0);
        const b2Vec2& contactPoint = worldManifold.points[0];
        const RefId id1 = s1->rigidbodyStub()->_ref->id(), id2 = s2->rigidbodyStub()->_ref->id();
        if(s1->_stub->_contacts.find(id2) == s1->_stub->_contacts.end())
        {
            s1->_stub->_contacts.insert(id2);
            s1->_rigidbody_stub->onBeginContact(s2->makeShadow(), CollisionManifold(V3(contactPoint.x, contactPoint.y, 0), normal));
        }
        if(s2->_stub->_contacts.find(id1) == s2->_stub->_contacts.end())
        {
            s2->_stub->_contacts.insert(id1);
            s2->_rigidbody_stub->onBeginContact(s1->makeShadow(), CollisionManifold(V3(contactPoint.x, contactPoint.y, 0), -normal));
        }
    }
}

void ColliderBox2D::ContactListenerImpl::EndContact(b2Contact* contact)
{
    RigidbodyBox2D* s1 = reinterpret_cast<RigidbodyBox2D*>(contact->GetFixtureA()->GetBody()->GetUserData().pointer);
    RigidbodyBox2D* s2 = reinterpret_cast<RigidbodyBox2D*>(contact->GetFixtureB()->GetBody()->GetUserData().pointer);
    if(s1 && s2)
    {
        const sp<RigidbodyBox2D::Stub>& body1 = s1->_stub;
        const sp<RigidbodyBox2D::Stub>& body2 = s2->_stub;
        const RefId id1 = s1->rigidbodyStub()->_ref->id(), id2 = s2->rigidbodyStub()->_ref->id();
        if(const auto it1 = body1->_contacts.find(id2); it1 != body1->_contacts.end())
        {
            body1->_contacts.erase(it1);
            s1->_rigidbody_stub->onEndContact(s2->makeShadow());
        }
        if(const auto it2 = body2->_contacts.find(id1); it2 != body2->_contacts.end())
        {
            body2->_contacts.erase(it2);
            s2->_rigidbody_stub->onEndContact(s1->makeShadow());
        }
    }
}

void ColliderBox2D::DestructionListenerImpl::SayGoodbye(b2Joint* joint)
{
    auto iter = _joints.find(joint);
    if(iter != _joints.end())
    {
        const sp<Joint::Stub> obj = iter->second.lock();
        if(obj)
            obj->release();

        _joints.erase(iter);
    }
}

void ColliderBox2D::DestructionListenerImpl::SayGoodbye(b2Fixture* /*fixture*/)
{
}

void ColliderBox2D::DestructionListenerImpl::track(const sp<Joint::Stub>& joint)
{
    _joints[joint->_joint] = joint;
}

}
