#include "box2d/impl/collider_box2d.h"

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/bean_utils.h"
#include "core/util/boolean_type.h"
#include "core/util/log.h"

#include "graphics/base/bounds.h"
#include "graphics/base/quaternion.h"
#include "graphics/base/rect.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_manifold.h"
#include "app/base/raycast_manifold.h"
#include "app/inf/collision_callback.h"

#include "box2d/impl/rigid_body_box2d.h"
#include "box2d/impl/joint.h"
#include "box2d/impl/shapes/ball.h"
#include "box2d/impl/shapes/box.h"

namespace ark {
namespace plugin {
namespace box2d {

ColliderBox2D::ColliderBox2D(const b2Vec2& gravity, const V2& pixelPerMeter)
    : _stub(sp<Stub>::make(gravity, pixelPerMeter))
{
    const BodyCreateInfo box(sp<Box>::make(), 1.0f, 0.2f);
    _stub->_body_manifests[Collider::BODY_SHAPE_AABB] = box;
    _stub->_body_manifests[Collider::BODY_SHAPE_BALL] = BodyCreateInfo(sp<Ball>::make(), 1.0f, 0.2f);
    _stub->_body_manifests[Collider::BODY_SHAPE_BOX] = box;
    _stub->_world.SetContactListener(&_stub->_contact_listener);
    _stub->_world.SetDestructionListener(&_stub->_destruction_listener);
}

void ColliderBox2D::run()
{
    _stub->run();
}

sp<RigidBody> ColliderBox2D::createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate)
{
    const auto iter = _stub->_body_manifests.find(shape);
    DCHECK(iter != _stub->_body_manifests.end(), "RigidBody shape-id: %d not found", shape);
    const BodyCreateInfo& manifest = iter->second;
    const sp<RigidBodyBox2D> body = sp<RigidBodyBox2D>::make(*this, type, position, size, rotate ? rotate->theta() : sp<Numeric>::null(), manifest);
    if(rotate)
        body->setAngle(rotate->theta()->val());

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

std::vector<RayCastManifold> ColliderBox2D::rayCast(const V3& from, const V3& to)
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

b2Body* ColliderBox2D::createBody(Collider::BodyType type, const V3& position, const sp<Size>& size, const BodyCreateInfo& createInfo) const
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

int32_t ColliderBox2D::genRigidBodyId() const
{
    return ++_stub->_rigid_body_id_base;
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
    DWARN(_stub->_body_manifests.find(id) == _stub->_body_manifests.end(), "Overriding existing body: %d", id);
    _stub->_body_manifests[id] = bodyManifest;
}

ColliderBox2D::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext), _ppm(factory.ensureBuilder<Vec2>(manifest, "pixel-per-meter")),
      _gravity(factory.ensureBuilder<Vec2>(manifest, "gravity")), _disposed(factory.getBuilder<Boolean>(manifest, Constants::Attributes::DISPOSED))
{
    for(const document& i : _manifest->children("import"))
        _importers.push_back({_factory.ensureBuilder<RigidBodyImporter>(i), Documents::ensureAttribute(i, Constants::Attributes::SRC)});
}

sp<ColliderBox2D> ColliderBox2D::BUILDER_IMPL1::build(const Scope& args)
{
    const V2 g = _gravity->build(args)->val();
    b2Vec2 gravity(g.x(), g.y());
    const sp<ColliderBox2D> world = sp<ColliderBox2D>::make(gravity, _ppm->build(args)->val());
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

    Ark& ark = Ark::instance();
    for(const auto& i : _importers)
    {
        const sp<RigidBodyImporter> importer = i.first->build(args);
        importer->import(world, ark.openAsset(i.second));
    }

    const sp<Boolean> expired = _disposed->build(args);
    _resource_loader_context->renderController()->addPreRenderRunRequest(world->_stub, expired ? expired : BooleanType::__or__(_resource_loader_context->disposed(), sp<Boolean>::make<BooleanByWeakRef<ColliderBox2D::Stub>>(world->_stub, 1)));
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
    : _ppm_x(pixelPerMeter.x()), _ppm_y(pixelPerMeter.y()), _time_step(1.0f / 60.0f), _velocity_iterations(6), _position_iterations(2), _rigid_body_id_base(0), _world(gravity)
{
}

void ColliderBox2D::Stub::run()
{
    _world.Step(_time_step, _velocity_iterations, _position_iterations);
}

void ColliderBox2D::ContactListenerImpl::BeginContact(b2Contact* contact)
{
    RigidBodyBox2D::Shadow* s1 = reinterpret_cast<RigidBodyBox2D::Shadow*>(contact->GetFixtureA()->GetBody()->GetUserData());
    RigidBodyBox2D::Shadow* s2 = reinterpret_cast<RigidBodyBox2D::Shadow*>(contact->GetFixtureB()->GetBody()->GetUserData());
    if(s1 && s2)
    {
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold);
        const V3 normal = V3(worldManifold.normal.x, worldManifold.normal.y, 0);
        const sp<RigidBodyBox2D::Stub> body1 = s1->_body.ensure();
        const sp<RigidBodyBox2D::Stub> body2 = s2->_body.ensure();
        const b2Vec2& contactPoint = worldManifold.points[0];
        if(body1->_contacts.find(body2->_id) == body1->_contacts.end())
        {
            body1->_contacts.insert(body2->_id);
            if(body1->_callback->hasCallback())
                body1->_callback->onBeginContact(RigidBodyBox2D::obtain(s2), CollisionManifold(V3(contactPoint.x, contactPoint.y, 0), normal));
        }
        if(body2->_contacts.find(body1->_id) == body2->_contacts.end())
        {
            body2->_contacts.insert(body1->_id);
            if(body2->_callback->hasCallback())
                body2->_callback->onBeginContact(RigidBodyBox2D::obtain(s1), CollisionManifold(V3(contactPoint.x, contactPoint.y, 0), -normal));
        }
    }
}

void ColliderBox2D::ContactListenerImpl::EndContact(b2Contact* contact)
{
    RigidBodyBox2D::Shadow* s1 = reinterpret_cast<RigidBodyBox2D::Shadow*>(contact->GetFixtureA()->GetBody()->GetUserData());
    RigidBodyBox2D::Shadow* s2 = reinterpret_cast<RigidBodyBox2D::Shadow*>(contact->GetFixtureB()->GetBody()->GetUserData());
    if(s1 && s2)
    {
        const sp<RigidBodyBox2D::Stub> body1 = s1->_body.ensure();
        const sp<RigidBodyBox2D::Stub> body2 = s2->_body.ensure();
        const auto it1 = body1->_contacts.find(body2->_id);
        if(it1 != body1->_contacts.end())
        {
            body1->_contacts.erase(it1);
            if(body1->_callback->hasCallback())
                body1->_callback->onEndContact(RigidBodyBox2D::obtain(s2));
        }
        const auto it2 = body2->_contacts.find(body1->_id);
        if(it2 != body2->_contacts.end())
        {
            body2->_contacts.erase(it2);
            if(body2->_callback->hasCallback())
                body2->_callback->onEndContact(RigidBodyBox2D::obtain(s1));
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
}
}
