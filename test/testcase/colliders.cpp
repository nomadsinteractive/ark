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
        : _render_object(renderObject) {
    }

    virtual void onBeginContact(const sp<RigidBody>& rigidBody) override {
        const V2 p1 = _render_object->xy();
        const V2 p2 = rigidBody->position()->val();
        printf("onBeginContact: (%.2f, %.2f) - (%.2f, %.2f)\n", p1.x(), p1.y(), p2.x(), p2.y());
    }

    virtual void onEndContact(const sp<RigidBody>& rigidBody) override {
        const V2 p1 = _render_object->xy();
        const V2 p2 = rigidBody->position()->val();
        printf("onEndContact: (%.2f, %.2f) - (%.2f, %.2f)\n", p1.x(), p1.y(), p2.x(), p2.y());
    }

private:
    sp<RenderObject> _render_object;
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
        const sp<RigidBody> rigidBody = collider->createBody(Collider::BODY_TYPE_DYNAMIC, Collider::BODY_SHAPE_BOX, c001->position(), c001->size());
        const sp<RenderObject> c002 = resourceLoader->load<RenderObject>("c002");
        collider->createBody(Collider::BODY_TYPE_STATIC, Collider::BODY_SHAPE_BOX, c002->position(), c002->size());
        rigidBody->setCollisionCallback(sp<CollisionCallbackImpl>::make(c001));
        while(duration->val() < 5.0f) {
            applicationContext->renderController()->preUpdate();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        return 0;
    }
};

}
}

ark::unittest::TestCase* colliders_create()
{
    return new ark::unittest::CollidersTestCase();
}
