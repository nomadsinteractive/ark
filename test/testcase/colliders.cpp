#include <chrono>
#include <thread>

#include "test/base/test_case.h"
#include "test/base/ref_counter.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/thread.h"
#include "core/base/bean_factory.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/quaternion.h"
#include "graphics/base/v2.h"

#include "app/base/application_context.h"
#include "core/base/resource_loader.h"
#include "app/base/rigid_body.h"
#include "app/inf/collider.h"
#include "app/inf/collision_callback.h"
#include "app/inf/broad_phrase.h"
#include "app/traits/shape.h"
#include "core/types/ref.h"

#include "platform/platform.h"

namespace ark::unittest {

namespace {

class CollisionCallbackImpl final : public CollisionCallback, public RefCounter<CollisionCallbackImpl> {
public:
    CollisionCallbackImpl(const sp<RenderObject>& renderObject)
        : _render_object(renderObject), _contact_began(false), _contact_ended(false) {
    }

    virtual void onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold) override {
        const V3 p1 = _render_object->position()->val();
        const V3 p2 = rigidBody.position().val();
        printf("onBeginContact: (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f) rotation: %.2f\n", p1.x(), p1.y(), p1.z(), p2.x(), p2.y(), p2.z(), _render_object->transform()->rotation()->theta().val());
        _contact_began = true;
    }

    virtual void onEndContact(const RigidBody& rigidBody) override {
        const V3 p1 = _render_object->position()->val();
        const V3 p2 = rigidBody.position().val();
        printf("onEndContact: (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f) rotation: %.2f\n", p1.x(), p1.y(), p1.z(), p2.x(), p2.y(), p2.z(), _render_object->transform()->rotation()->theta().val());
        if(_contact_began)
            _contact_ended = true;
    }

    bool successed() const {
        return _contact_began && _contact_ended;
    }

private:
    sp<RenderObject> _render_object;
    bool _contact_began;
    bool _contact_ended;
};

}

class CollidersTestCase : public TestCase {
public:
    virtual int launch() override {
        const sp<ResourceLoader> resourceLoader = getResourceLoader();
        const sp<ApplicationContext>& applicationContext = Ark::instance().applicationContext();
        const sp<Clock>& clock = applicationContext->sysClock();
        const sp<Numeric> duration = clock->duration();

        Scope args;
        args.put("t", duration);
        const sp<Collider> collider = resourceLoader->load<Collider>("collider-01", args);
        const sp<BroadPhrase> bp1 = resourceLoader->load<BroadPhrase>("tracker-01", args);
        const sp<BroadPhrase> bp2 = resourceLoader->load<BroadPhrase>("tracker-02", args);

        bp2->create(1, V3(0), V3(600));
        bp2->create(1, V3(4, 4, 0), V3(600));
        const auto r = bp2->search(V3(220, 0, 0), V3(16, 32, 0));

        const sp<RenderObject> c001 = resourceLoader->load<RenderObject>("c001", args);
        const sp<RigidBody> rigidBody001 = collider->createBody(Collider::BODY_TYPE_DYNAMIC, sp<Shape>::make(Shape::SHAPE_ID_AABB, static_cast<sp<Vec3>>(c001->size())), c001->position());
        const sp<RenderObject> c002 = resourceLoader->load<RenderObject>("c002", args);
        const sp<RigidBody> rigidBody002 = collider->createBody(Collider::BODY_TYPE_STATIC, sp<Shape>::make(Shape::SHAPE_ID_AABB, static_cast<sp<Vec3>>(c002->size())), c002->position());
        const sp<RenderObject> c003 = resourceLoader->load<RenderObject>("c003", args);
        const sp<RigidBody> rigidBody003 = collider->createBody(Collider::BODY_TYPE_DYNAMIC, sp<Shape>::make(1, static_cast<sp<Vec3>>(c003->size())), c003->position(), c003->transform()->rotation());
        const sp<CollisionCallbackImpl> collisionCallbackImpl001 = sp<CollisionCallbackImpl>::make(c001);
        const sp<CollisionCallbackImpl> collisionCallbackImpl003 = sp<CollisionCallbackImpl>::make(c003);
        rigidBody001->setCollisionCallback(collisionCallbackImpl001);
        rigidBody003->setCollisionCallback(collisionCallbackImpl003);
        const auto s1 = bp1->search(V3(120, 380, 0), V3(60, 60, 0))._dynamic_candidates;
        TESTCASE_VALIDATE(s1.find(rigidBody001->ref()->id()) != s1.end());
        const auto s2 = bp1->search(V3(40, 380, 0), V3(60, 60, 0))._dynamic_candidates;
        TESTCASE_VALIDATE(s2.find(rigidBody001->ref()->id()) == s2.end());
        const auto s3 = bp1->search(V3(120, 450, 0), V3(60, 60, 0))._dynamic_candidates;
        TESTCASE_VALIDATE(s3.find(rigidBody001->ref()->id()) == s3.end());

        while(duration->val() < 3.0f) {
            applicationContext->updateRenderState();
            applicationContext->renderController()->onPreCompose(clock->tick());
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        TESTCASE_VALIDATE(collisionCallbackImpl001->successed());
        TESTCASE_VALIDATE(collisionCallbackImpl003->successed());
        return 0;
    }
};

}

ark::unittest::TestCase* colliders_create()
{
    return new ark::unittest::CollidersTestCase();
}
