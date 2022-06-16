#include "app/impl/collider/collider_impl.h"

#include <algorithm>
#include <iterator>

#include "core/types/null.h"
#include "core/util/log.h"
#include "core/util/variable_util.h"

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
    DynamicPosition(const sp<ColliderImpl::Stub>& collider, int32_t rigidBodyId, sp<Vec3> position, sp<Vec3> size)
        : _collider(collider), _rigid_body_id(rigidBodyId), _position(std::move(position)), _size(std::move(size)), _position_updated(true), _size_updated(false) {
    }

    virtual V3 val() override {
        const V3 position = _position->val();
        sp<ColliderImpl::RigidBodyShadow> rigidBody = _collider->findRigidBody(_rigid_body_id);
        if(rigidBody) {
            const V3 size = _size->val();
            if(_size_updated || _position_updated) {
                if(_size_updated) {
                    rigidBody->updateShapes(_collider->narrowPhrase(), size);
                    _size_updated = false;
                }
                float s = std::max(size.x(), size.y());
                _collider->updateBroadPhraseCandidate(_rigid_body_id, position, V3(s, s, 0));
                _position_updated = false;
            }
            rigidBody->collision(rigidBody, _collider, position, size);
        }
        return position;
    }

    virtual bool update(uint64_t timestamp) override {
        _size_updated = _size_updated | _size->update(timestamp);
        _position_updated = _position_updated | _position->update(timestamp);
        return true;
    }

private:
    sp<ColliderImpl::Stub> _collider;
    int32_t _rigid_body_id;
    sp<Vec3> _position;
    sp<Vec3> _size;
    bool _position_updated;
    bool _size_updated;
};

}

ColliderImpl::ColliderImpl(std::vector<sp<BroadPhrase>> broadPhrase, sp<NarrowPhrase> narrowPhrase, const document& manifest, const sp<RenderController>& renderController)
    : _stub(sp<Stub>::make(std::move(broadPhrase), std::move(narrowPhrase))), _render_controller(renderController)
{
}

sp<RigidBody> ColliderImpl::createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate)
{
    DASSERT(position);
    DASSERT(size);
    DCHECK(type == Collider::BODY_TYPE_KINEMATIC || type == Collider::BODY_TYPE_DYNAMIC || type == Collider::BODY_TYPE_STATIC || type == Collider::BODY_TYPE_SENSOR, "Unknown BodyType: %d", type);

    const sp<Disposed> disposed = sp<Disposed>::make();
    const int32_t rigidBodyId = _stub->generateRigidBodyId();
    if(type != Collider::BODY_TYPE_STATIC)
    {
        sp<DynamicPosition> dpos = sp<DynamicPosition>::make(_stub, rigidBodyId, position, size);
        sp<Vec3> pos = _render_controller->synchronize<V3>(std::move(dpos), disposed);
        return _stub->createRigidBody(rigidBodyId, type, shape, pos, size, rotate, disposed, _stub);
    }
    return _stub->createRigidBody(rigidBodyId, type, shape, position, size, rotate, disposed, _stub);
}

std::vector<RayCastManifold> ColliderImpl::rayCast(const V3& from, const V3& to)
{
    return _stub->rayCast(V2(from.x(), from.y()), V2(to.x(), to.y()));
}

ColliderImpl::Stub::Stub(std::vector<sp<BroadPhrase>> broadPhrases, sp<NarrowPhrase> narrowPhrase)
    : _rigid_body_base_id(0), _broad_phrases(std::move(broadPhrases)), _narrow_phrase(std::move(narrowPhrase))
{
}

BroadPhrase::Result ColliderImpl::Stub::broadPhraseSearch(const V3& position, const V3& aabb) const
{
    if(_broad_phrases.size() == 1)
        return _broad_phrases.at(0)->search(position, aabb);

    BroadPhrase::Result result;
    for(const sp<BroadPhrase>& i : _broad_phrases)
        result.merge(i->search(position, aabb));

    return result;
}

BroadPhrase::Result ColliderImpl::Stub::broadPhraseRayCast(const V3& from, const V3& to) const
{
    if(_broad_phrases.size() == 1)
        return _broad_phrases.at(0)->rayCast(from, to);

    BroadPhrase::Result result;
    for(const sp<BroadPhrase>& i : _broad_phrases)
        result.merge(i->rayCast(from, to));

    return result;
}

void ColliderImpl::Stub::updateBroadPhraseCandidate(int32_t id, const V3& position, const V3& aabb)
{
    for(const sp<BroadPhrase>& i : _broad_phrases)
        i->update(id, position, aabb);
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

int32_t ColliderImpl::Stub::generateRigidBodyId()
{
    return ++_rigid_body_base_id;
}

sp<ColliderImpl::RigidBodyImpl> ColliderImpl::Stub::createRigidBody(int32_t rigidBodyId, Collider::BodyType type, int32_t shape, sp<Vec3> position, sp<Size> size, sp<Rotation> rotate, sp<Disposed> disposed, sp<ColliderImpl::Stub> self)
{
    const V3 posVal = position->val();
    const V3 sizeVal = size->val();
    sp<RigidBodyShadow> rigidBodyShadow = sp<RigidBodyShadow>::make(rigidBodyId, type, shape, std::move(position), std::move(size), std::move(rotate), std::move(disposed));
    rigidBodyShadow->updateShapes(_narrow_phrase, sizeVal);
    _rigid_bodies[rigidBodyShadow->id()] = rigidBodyShadow;

    float s = std::max(sizeVal.x(), sizeVal.y());
    for(const sp<BroadPhrase>& i : _broad_phrases)
        i->create(rigidBodyId, posVal, V3(s, s, s));

    return sp<RigidBodyImpl>::make(self, std::move(rigidBodyShadow));
}

const sp<ColliderImpl::RigidBodyShadow>& ColliderImpl::Stub::ensureRigidBody(int32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(id = %d) does not exists", id);
    return iter->second;
}

sp<ColliderImpl::RigidBodyShadow> ColliderImpl::Stub::ensureRigidBody(int32_t id, int32_t shapeId, const V3& position, bool isDynamicCandidate) const
{
    sp<RigidBodyShadow> s = isDynamicCandidate ? findRigidBody(id) : nullptr;
    return s ? s : sp<RigidBodyShadow>::make(id, isDynamicCandidate ? Collider::BODY_TYPE_DYNAMIC : Collider::BODY_TYPE_STATIC, shapeId, sp<Vec3::Const>::make(position), nullptr, nullptr, nullptr);
}

sp<ColliderImpl::RigidBodyShadow> ColliderImpl::Stub::findRigidBody(int32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    return iter != _rigid_bodies.end() ? iter->second : sp<RigidBodyShadow>();
}

std::vector<sp<ColliderImpl::RigidBodyShadow>> ColliderImpl::Stub::toRigidBodyShadows(const std::unordered_set<int32_t>& candidateSet, uint32_t filter) const
{
    std::vector<sp<ColliderImpl::RigidBodyShadow>> rigidBodies;
    for(int32_t i : candidateSet)
    {
        sp<RigidBodyShadow> rigidBody = ensureRigidBody(i);
        if(rigidBody->type() & filter)
            rigidBodies.push_back(std::move(rigidBody));
    }
    return rigidBodies;
}

std::vector<BroadPhrase::Candidate> ColliderImpl::Stub::toBroadPhraseCandidates(const std::unordered_set<int32_t>& candidateSet, uint32_t filter) const
{
    std::vector<BroadPhrase::Candidate> candidates;
    for(int32_t i : candidateSet)
    {
        const sp<RigidBodyShadow>& rigidBody = ensureRigidBody(i);
        if(rigidBody->type() & filter)
            candidates.emplace_back(toBroadPhraseCandidate(rigidBody));
    }
    return candidates;
}

BroadPhrase::Candidate ColliderImpl::Stub::toBroadPhraseCandidate(const RigidBodyShadow& rigidBody) const
{
    return BroadPhrase::Candidate(rigidBody.id(), rigidBody.position()->val(), rigidBody.transform()->rotation()->theta()->val(), rigidBody.shapeId(), rigidBody.collisionFilter(), rigidBody.shapes());
}

std::vector<RayCastManifold> ColliderImpl::Stub::rayCast(const V2& from, const V2& to) const
{
    std::vector<RayCastManifold> manifolds;
    const BroadPhrase::Result result = broadPhraseRayCast(V3(from.x(), from.y(), 0), V3(to.x(), to.y(), 0));

    const NarrowPhrase::Ray ray = _narrow_phrase->toRay(from, to);
    for(const auto& i : toRigidBodyShadows(result.dynamic_candidates, Collider::BODY_TYPE_RIGID))
    {
        RayCastManifold raycast(0, V3(0), i);
        if(_narrow_phrase->rayCastManifold(ray, toBroadPhraseCandidate(*i), raycast))
            manifolds.push_back(raycast);
    }
    RayCastManifold raycast;
    for(const auto& i : result.static_candidates)
    {
        if(_narrow_phrase->rayCastManifold(ray, i, raycast))
            manifolds.emplace_back(raycast.distance(), raycast.normal(), sp<RigidBodyShadow>::make(i.id, Collider::BODY_TYPE_STATIC, i.shape_id, sp<Vec3::Const>::make(V3(i.position.x(), i.position.y(), 0)), nullptr, nullptr, nullptr));
    }
    return manifolds;
}

void ColliderImpl::Stub::resolveCandidates(const sp<RigidBody>& self, const BroadPhrase::Candidate& candidateSelf, const std::vector<BroadPhrase::Candidate>& candidates, bool isDynamicCandidates, RigidBody::Callback& callback, std::set<int32_t>& c)
{
    std::set<int32_t> contacts = std::move(c);
    std::set<int32_t> contactsOut;
    for(const BroadPhrase::Candidate& i : candidates)
    {
        CollisionManifold manifold;
        if(_narrow_phrase->collisionManifold(candidateSelf, i, manifold))
        {
            auto iter = contacts.find(i.id);
            if(iter == contacts.end())
                callback.onBeginContact(self, ensureRigidBody(i.id, i.shape_id, V3(i.position.x(), i.position.y(), 0), isDynamicCandidates), manifold);
            else
                contacts.erase(iter);
            contactsOut.insert(i.id);
        }
    }
    for(int32_t i : contacts)
    {
        if(contactsOut.find(i) == contactsOut.end())
            callback.onEndContact(self, ensureRigidBody(i, 0, V3(), isDynamicCandidates));
    }
    c = std::move(contactsOut);
}

const sp<NarrowPhrase>& ColliderImpl::Stub::narrowPhrase() const
{
    return _narrow_phrase;
}

ColliderImpl::RigidBodyImpl::RigidBodyImpl(sp<ColliderImpl::Stub> collider, sp<RigidBodyShadow> shadow)
    : RigidBody(shadow->stub()), _collider(std::move(collider)), _shadow(std::move(shadow))
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

ColliderImpl::RigidBodyShadow::RigidBodyShadow(uint32_t id, Collider::BodyType type, int32_t shapeId, sp<Vec3> position, sp<Size> size, sp<Rotation> rotate, sp<Disposed> disposed)
    : RigidBody(id, type, shapeId, std::move(position), std::move(size), std::move(rotate), Box(), std::move(disposed)), _dispose_requested(false)
{
}

void ColliderImpl::RigidBodyShadow::dispose()
{
    _dispose_requested = true;
}

void ColliderImpl::RigidBodyShadow::collision(const sp<RigidBodyShadow>& self, ColliderImpl::Stub& collider, const V3& position, const V3& size)
{
    if(_dispose_requested)
    {
        doDispose(collider);
        return;
    }

    const BroadPhrase::Result result = collider.broadPhraseSearch(position, size);
    std::unordered_set<int32_t> dynamicCandidates = std::move(result.dynamic_candidates);
    const Stub& shadowStub = stub();
    const auto iter = dynamicCandidates.find(shadowStub._id);
    if(iter != dynamicCandidates.end())
        dynamicCandidates.erase(iter);

    const BroadPhrase::Candidate candidateSelf(self->id(), position, self->transform()->rotation()->theta()->val(), self->shapeId(), self->collisionFilter(), self->shapes());
    collider.resolveCandidates(self, candidateSelf, collider.toBroadPhraseCandidates(dynamicCandidates, Collider::BODY_TYPE_ALL), true, shadowStub._callback, _dynamic_contacts);
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

const std::vector<Box>& ColliderImpl::RigidBodyShadow::shapes() const
{
    return _shapes;
}

void ColliderImpl::RigidBodyShadow::updateShapes(NarrowPhrase& narrowPhrase, const V3& size)
{
    const Rect bounds(size.x() / -2.0f, size.y() / -2.0f, size.x() / 2.0f, size.y() / 2.0f);
    switch(shapeId())
    {
    case Collider::BODY_SHAPE_BALL:
        _shapes = { narrowPhrase.makeBallShape(V2(0, 0), size.x()) };
        break;
    case Collider::BODY_SHAPE_AABB:
    {
        _shapes = { narrowPhrase.makeAABBShape(bounds) };
        break;
    }
    case Collider::BODY_SHAPE_CAPSULE:
    {
        float radius = bounds.width() / 2;
        float x = (bounds.left() + bounds.right()) / 2;
        DCHECK(radius < bounds.height() / 2, "Capsule too narrow, width = %.2f, height = %.2f, radius = %.2f", bounds.width(), bounds.height(), radius);
        _shapes = { narrowPhrase.makeCapsuleShape(V2(x, bounds.top() + radius), V2(x, bounds.bottom() - radius), radius) };
        break;
    }
    case Collider::BODY_SHAPE_BOX:
        _shapes = { narrowPhrase.makeBoxShape(bounds) };
        break;
    default:
        break;
    }
}

bool ColliderImpl::RigidBodyShadow::isDisposed() const
{
    return disposed()->val();
}

ColliderImpl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _broad_phrases(factory.getBuilderList<BroadPhrase>(manifest, "broad-phrase")), _narrow_phrase(factory.ensureBuilder<NarrowPhrase>(manifest, "narrow-phrase")),
      _render_controller(resourceLoaderContext->renderController())
{
    DCHECK(_broad_phrases.size() > 0, "Collider should have at least one BroadPhrase");
}

sp<Collider> ColliderImpl::BUILDER::build(const Scope& args)
{
    std::vector<sp<BroadPhrase>> broadPhrases;
    for(const auto& i : _broad_phrases)
        broadPhrases.push_back(i->build(args));
    return sp<ColliderImpl>::make(std::move(broadPhrases), _narrow_phrase->build(args), _manifest, _render_controller);
}

}
