#include "app/impl/collider/collider_impl.h"

#include <algorithm>
#include <iterator>

#include "core/types/null.h"
#include "core/util/log.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/base/raycast_manifold.h"
#include "app/base/rigid_body.h"
#include "app/inf/broad_phrase.h"
#include "app/inf/narrow_phrase.h"
#include "app/inf/collision_callback.h"

namespace ark {

namespace {

class DynamicPosition : public Vec3 {
public:
    DynamicPosition(const sp<ColliderImpl::Stub>& collider, const sp<Vec3>& position)
        : _collider(collider), _position(position) {
    }

    void setRigidBody(const sp<ColliderImpl::RigidBodyShadow>& rigidBody) {
        _rigid_body_shadow = rigidBody;
        _size = _rigid_body_shadow->size()->val();
    }

    virtual V3 val() override {
        const V3 position = _position->val();
        if(_rigid_body_shadow && _rigid_body_shadow->isDisposed())
            _rigid_body_shadow = nullptr;
        if(_rigid_body_shadow) {
            _rigid_body_shadow->collision(_rigid_body_shadow, _collider, position, _size);
        }
        return position;
    }

    virtual bool update(uint64_t timestamp) override {
        return _position->update(timestamp);
    }

private:
    sp<ColliderImpl::Stub> _collider;
    sp<Vec3> _position;
    V3 _size;
    sp<ColliderImpl::RigidBodyShadow> _rigid_body_shadow;
};

}

ColliderImpl::ColliderImpl(std::vector<sp<BroadPhrase>> broadPhrase, sp<NarrowPhrase> narrowPhrase, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _stub(sp<Stub>::make(std::move(broadPhrase), std::move(narrowPhrase), manifest, resourceLoaderContext)), _resource_loader_context(resourceLoaderContext)
{
}

ColliderImpl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _broad_phrases(factory.getBuilderList<BroadPhrase>(manifest, "broad-phrase")),
      _narrow_phrase(factory.ensureBuilder<NarrowPhrase>(manifest, "narrow-phrase"))
{
    DCHECK(_broad_phrases.size() > 0, "Collider should have at least one BroadPhrase");
}

sp<Collider> ColliderImpl::BUILDER::build(const Scope& args)
{
    std::vector<sp<BroadPhrase>> broadPhrases;
    for(const auto& i : _broad_phrases)
        broadPhrases.push_back(i->build(args));
    return sp<ColliderImpl>::make(std::move(broadPhrases), _narrow_phrase->build(args), _manifest, _resource_loader_context);
}

sp<RigidBody> ColliderImpl::createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate)
{
    DASSERT(position);
    DASSERT(size);
    DCHECK(type == Collider::BODY_TYPE_KINEMATIC || type == Collider::BODY_TYPE_DYNAMIC || type == Collider::BODY_TYPE_STATIC || type == Collider::BODY_TYPE_SENSOR, "Unknown BodyType: %d", type);

    const sp<Disposed> disposed = sp<Disposed>::make();
    if(type != Collider::BODY_TYPE_STATIC)
    {
        const sp<DynamicPosition> dpos = sp<DynamicPosition>::make(_stub, position);
        const sp<Vec3> pos = _resource_loader_context->synchronize<V3>(dpos, disposed);
        const sp<RigidBodyImpl> rigidBody = _stub->createRigidBody(type, shape, pos, size, rotate, disposed, _stub);
        dpos->setRigidBody(rigidBody->shadow());
        return rigidBody;
    }
    return _stub->createRigidBody(type, shape, position, size, rotate, disposed, _stub);
}

std::vector<RayCastManifold> ColliderImpl::rayCast(const V3& from, const V3& to)
{
    return _stub->rayCast(V2(from.x(), from.y()), V2(to.x(), to.y()));
}

ColliderImpl::Stub::Stub(std::vector<sp<BroadPhrase>> broadPhrases, sp<NarrowPhrase> narrowPhrase, const document& manifest, ResourceLoaderContext& resourceLoaderContext)
    : _rigid_body_base_id(0), _broad_phrases(std::move(broadPhrases)), _narrow_phrase(std::move(narrowPhrase)), _static_rigid_body_shadow(sp<RigidBodyShadow>::make(0, Collider::BODY_TYPE_STATIC, nullptr, nullptr, nullptr, nullptr))
{
}

BroadPhrase::Result ColliderImpl::Stub::search(const V3& position, const V3& aabb) const
{
    if(_broad_phrases.size() == 1)
        return _broad_phrases.at(0)->search(position, aabb);

    BroadPhrase::Result result;
    for(const sp<BroadPhrase>& i : _broad_phrases)
        result.merge(i->search(position, aabb));

    return result;
}

sp<Vec3> ColliderImpl::Stub::createBroadPhrasePosition(int32_t id, const sp<Vec3>& position, const sp<Vec3>& aabb)
{
    sp<Vec3> pos = position;
    for(const sp<BroadPhrase>& i : _broad_phrases)
        pos = i->create(id, pos, aabb);
    return pos;
}

void ColliderImpl::Stub::remove(const RigidBody& rigidBody)
{
    for(const sp<BroadPhrase>& i : _broad_phrases)
        i->remove(rigidBody.id());
    const auto iter = _rigid_bodies.find(rigidBody.id());
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(%d) not found", rigidBody.id());
    LOGD("Removing RigidBody(%d)", rigidBody.id());
    _rigid_bodies.erase(iter);
}

sp<ColliderImpl::RigidBodyImpl> ColliderImpl::Stub::createRigidBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate, const sp<Disposed>& disposed, const sp<ColliderImpl::Stub>& self)
{
    int32_t rigidBodyId = ++_rigid_body_base_id;
    float s = std::max(size->width(), size->height());
    const sp<Vec3> dp = createBroadPhrasePosition(rigidBodyId, position, sp<Vec3::Const>::make(V3(s)));
    const sp<RigidBodyShadow> rigidBodyShadow = sp<RigidBodyShadow>::make(rigidBodyId, type, dp, size, rotate, disposed);
    const sp<RigidBodyImpl> rigidBody = sp<RigidBodyImpl>::make(self, rigidBodyShadow);

    switch(shape)
    {
    case BODY_SHAPE_AABB:
        addAABBShape(rigidBodyId, rigidBodyShadow);
        break;
    case BODY_SHAPE_BALL:
        addBallShape(rigidBodyId, rigidBodyShadow);
        break;
    case BODY_SHAPE_BOX:
        addBoxShape(rigidBodyId, rigidBodyShadow);
        break;
    case BODY_SHAPE_CAPSULE:
        addCapsuleShape(rigidBodyId, rigidBodyShadow);
        break;
    default:
        break;
    }

    _rigid_bodies[rigidBodyShadow->id()] = rigidBodyShadow;
    return rigidBody;
}

const sp<ColliderImpl::RigidBodyShadow>& ColliderImpl::Stub::ensureRigidBody(int32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(id = %d) does not exists", id);
    return iter->second;
}

const sp<ColliderImpl::RigidBodyShadow> ColliderImpl::Stub::ensureRigidBody(int32_t id, bool isDynamicCandidate) const
{
    sp<RigidBodyShadow> s = isDynamicCandidate ? findRigidBody(id) : nullptr;
    if(!isDynamicCandidate && !s)
    {
        _static_rigid_body_shadow->setId(id);
        return _static_rigid_body_shadow;
    }
    return s ? s : sp<RigidBodyShadow>::make(id, isDynamicCandidate ? Collider::BODY_TYPE_DYNAMIC : Collider::BODY_TYPE_STATIC, nullptr, nullptr, nullptr, nullptr);
}

const sp<ColliderImpl::RigidBodyShadow> ColliderImpl::Stub::findRigidBody(int32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    return iter != _rigid_bodies.end() ? iter->second : sp<RigidBodyShadow>();
}

std::vector<BroadPhrase::Candidate> ColliderImpl::Stub::toDynamicCandidates(const std::unordered_set<int32_t>& candidateSet) const
{
    std::vector<BroadPhrase::Candidate> candidates;
    for(int32_t i : candidateSet)
    {
        const sp<RigidBodyShadow>& rigidBody = ensureRigidBody(i);
        candidates.emplace_back(i, rigidBody->position()->val(), rigidBody->transform()->rotation()->theta()->val(), i);
    }
    return candidates;
}

std::vector<RayCastManifold> ColliderImpl::Stub::rayCast(const V2& from, const V2& to) const
{
    std::vector<RayCastManifold> manifolds;
    const NarrowPhrase::Ray ray = _narrow_phrase->toRay(from, to);
    const BroadPhrase::Result result = _broad_phrases.at(0)->rayCast(V3(from.x(), from.y(), 0), V3(to.x(), to.y(), 0));
    for(const auto& i : toDynamicCandidates(result.dynamic_candidates))
    {
        RayCastManifold raycast;
        if(_narrow_phrase->rayCastManifold(ray, i, raycast))
            manifolds.push_back(raycast);
    }
    return manifolds;
}

void ColliderImpl::Stub::resolveCandidates(const sp<RigidBody>& self, const BroadPhrase::Candidate& candidateSelf, const std::vector<BroadPhrase::Candidate>& candidates, bool isDynamicCandidates, RigidBody::Callback& callback, std::set<int32_t>& c)
{
    std::set<int32_t> contacts = c;
    std::set<int32_t> contactsOut;
    for(const auto& i : candidates)
    {
        CollisionManifold manifold;
        if(_narrow_phrase->collisionManifold(candidateSelf, i, manifold))
        {
            auto iter2 = contacts.find(i.id);
            if(iter2 == contacts.end())
                callback.onBeginContact(self, ensureRigidBody(i.id, isDynamicCandidates), manifold);
            else
                contacts.erase(iter2);
            contactsOut.insert(i.id);
        }
    }
    for(int32_t i : contacts)
    {
        if(contactsOut.find(i) == contactsOut.end())
            callback.onEndContact(self, ensureRigidBody(i, isDynamicCandidates));
    }
    c = std::move(contactsOut);
}

void ColliderImpl::Stub::addAABBShape(uint32_t id, const RigidBodyShadow& rigidBody)
{
    _narrow_phrase->addAABBShape(id, rigidBody.getRigidBodyAABB());
}

void ColliderImpl::Stub::addBoxShape(uint32_t id, const RigidBodyShadow& rigidBody)
{
    _narrow_phrase->addBoxShape(id, rigidBody.getRigidBodyAABB());
}

void ColliderImpl::Stub::addBallShape(uint32_t id, const RigidBodyShadow& rigidBody)
{
    const Rect bounds = rigidBody.getRigidBodyAABB();
    _narrow_phrase->addBallShape(id, V2((bounds.left() + bounds.right()) / 2, (bounds.top() + bounds.bottom()) / 2), bounds.width() / 2);
}

void ColliderImpl::Stub::addCapsuleShape(uint32_t id, const RigidBodyShadow& rigidBody)
{
    const Rect aabb = rigidBody.getRigidBodyAABB();
    float radius = aabb.width() / 2;
    float x = (aabb.left() + aabb.right()) / 2;
    DCHECK(radius < aabb.height() / 2, "Capsule too narrow, width = %.2f, height = %.2f, radius = %.2f", aabb.width(), aabb.height(), radius);
    _narrow_phrase->addCapsuleShape(id, V2(x, aabb.top() + radius), V2(x, aabb.bottom() - radius), radius);
}

ColliderImpl::RigidBodyImpl::RigidBodyImpl(const sp<ColliderImpl::Stub>& collider, const sp<RigidBodyShadow>& shadow)
    : RigidBody(shadow->stub()), _collider(collider), _shadow(shadow)
{
}

ColliderImpl::RigidBodyImpl::~RigidBodyImpl()
{
    doDispose();
}

const sp<ColliderImpl::RigidBodyShadow>& ColliderImpl::RigidBodyImpl::shadow() const
{
    return _shadow;
}

void ColliderImpl::RigidBodyImpl::doDispose()
{
    _shadow->doDispose(_collider);
}

void ColliderImpl::RigidBodyImpl::dispose()
{
    doDispose();
}

ColliderImpl::RigidBodyShadow::RigidBodyShadow(uint32_t id, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate, const sp<Disposed>& disposed)
    : RigidBody(id, type, position, size, rotate, Box(), disposed), _is_static(type == Collider::BODY_TYPE_STATIC), _dispose_requested(false)
{
}

void ColliderImpl::RigidBodyShadow::dispose()
{
    _dispose_requested = true;
}

bool ColliderImpl::RigidBodyShadow::isStatic() const
{
    return _is_static;
}

void ColliderImpl::RigidBodyShadow::setId(uint32_t id)
{
    stub()->_id = id;
}

void ColliderImpl::RigidBodyShadow::collision(const sp<RigidBodyShadow>& self, ColliderImpl::Stub& collider, const V3& position, const V3& size)
{
    if(_dispose_requested)
    {
        doDispose(collider);
        return;
    }

    const BroadPhrase::Result result = collider.search(position, size);
    std::unordered_set<int32_t> dynamicCandidates = std::move(result.dynamic_candidates);
    const Stub& shadowStub = stub();
    const auto iter = dynamicCandidates.find(shadowStub._id);
    if(iter != dynamicCandidates.end())
        dynamicCandidates.erase(iter);

    const BroadPhrase::Candidate candidateSelf(self->id(), position, self->transform()->rotation()->theta()->val(), self->id());
    collider.resolveCandidates(self, candidateSelf, collider.toDynamicCandidates(dynamicCandidates), true, shadowStub._callback, _dynamic_contacts);
    collider.resolveCandidates(self, candidateSelf, result.static_candidates, false, shadowStub._callback, _static_contacts);
}

void ColliderImpl::RigidBodyShadow::doDispose(ColliderImpl::Stub& stub)
{
    if(!isDisposed())
    {
        stub.remove(*this);
        disposed()->dispose();
    }
}

bool ColliderImpl::RigidBodyShadow::isDisposed() const
{
    return disposed()->val();
}

Rect ColliderImpl::RigidBodyShadow::getRigidBodyAABB() const
{
    DCHECK(stub()->_size, "RigidBody must have size defined");
    Rect aabb(0, 0, stub()->_size->width(), stub()->_size->height());
    if(_is_static)
    {
        const V2 pos = stub()->_position->val();
        aabb.setCenter(pos.x(), pos.y());
    }
    else
        aabb.setCenter(0, 0);
    return aabb;
}

}
