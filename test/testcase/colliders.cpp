#include <chrono>
#include <thread>

#include "test/base/test_case.h"
#include "test/base/ref_count.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/thread.h"

#include "core/base/bean_factory.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/v2.h"

#include "app/base/application_context.h"
#include "app/base/resource_loader.h"
#include "app/base/rigid_body.h"
#include "app/inf/collider.h"
#include "app/inf/collision_callback.h"

namespace ark {
namespace unittest {

namespace {

class CollisionCallbackImpl : public CollisionCallback, public RefCount<CollisionCallbackImpl> {
public:
    CollisionCallbackImpl(const sp<RenderObject>& renderObject)
        : _render_object(renderObject), _contact_began(false), _contact_ended(false) {
    }

    virtual void onBeginContact(const sp<RigidBody>& rigidBody) override {
        const V p1 = _render_object->position()->val();
        const V p2 = rigidBody->position()->val();
        printf("onBeginContact: (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f) rotation: %.2f\n", p1.x(), p1.y(), p1.z(), p2.x(), p2.y(), p2.z(), _render_object->transform()->rotation()->val());
        _contact_began = true;
    }

    virtual void onEndContact(const sp<RigidBody>& rigidBody) override {
        const V p1 = _render_object->xy();
        const V p2 = rigidBody->position()->val();
        printf("onEndContact: (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f) rotation: %.2f\n", p1.x(), p1.y(), p1.z(), p2.x(), p2.y(), p2.z(), _render_object->transform()->rotation()->val());
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
        const sp<Clock>& clock = applicationContext->clock();
        const sp<Numeric> duration = clock->duration();
        const sp<Collider> collider = resourceLoader->load<Collider>("collider-001");
        const sp<RenderObject> c001 = resourceLoader->load<RenderObject>("c001");
        const sp<RigidBody> rigidBody001 = collider->createBody(Collider::BODY_TYPE_DYNAMIC, Collider::BODY_SHAPE_AABB, c001->position(), c001->size());
        const sp<RenderObject> c002 = resourceLoader->load<RenderObject>("c002");
        const sp<RigidBody> rigidBody002 = collider->createBody(Collider::BODY_TYPE_STATIC, Collider::BODY_SHAPE_AABB, c002->position(), c002->size());
        const sp<RenderObject> c003 = resourceLoader->load<RenderObject>("c003");
        const sp<RigidBody> rigidBody003 = collider->createBody(Collider::BODY_TYPE_DYNAMIC, 0, c003->position(), c003->size(), c003->transform());
        const sp<CollisionCallbackImpl> collisionCallbackImpl001 = sp<CollisionCallbackImpl>::make(c001);
        const sp<CollisionCallbackImpl> collisionCallbackImpl003 = sp<CollisionCallbackImpl>::make(c003);
        rigidBody001->setCollisionCallback(collisionCallbackImpl001);
        rigidBody003->setCollisionCallback(collisionCallbackImpl003);
        while(duration->val() < 3.0f) {
            applicationContext->renderController()->preUpdate();
            rigidBody001->xy();
            rigidBody003->xy();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        if(!collisionCallbackImpl001->successed())
            return 1;
        if(!collisionCallbackImpl003->successed())
            return 2;
        return 0;
    }
};

}
}

ark::unittest::TestCase* colliders_create()
{
    return new ark::unittest::CollidersTestCase();
}
