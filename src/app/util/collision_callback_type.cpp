#include "collision_callback_type.h"

#include "app/base/application_context.h"
#include "core/inf/interpreter.h"
#include "core/components/behavior.h"

#include "app/base/collision_manifold.h"
#include "app/base/rigidbody.h"
#include "app/inf/collision_callback.h"

namespace ark {

namespace {

class CollisionCallbackImpl : public CollisionCallback {
public:
    CollisionCallbackImpl(sp<Behavior::Method> onBeginContact, sp<Behavior::Method> onEndContact)
        : _on_begin_contact(std::move(onBeginContact)), _on_end_contact(std::move(onEndContact))
    {
    }

    void onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold) override
    {
        if(_on_begin_contact)
        {
            Box arg1(rigidBody.makeShadow());
            Box arg2(sp<CollisionManifold>::make(manifold));
            _on_begin_contact->call({std::move(arg1), std::move(arg2)});
        }
    }

    void onEndContact(const Rigidbody& rigidBody) override
    {
        if(_on_begin_contact)
        {
            Box arg1(rigidBody.makeShadow());
            _on_end_contact->call({std::move(arg1)});
        }
    }

private:
    sp<Behavior::Method> _on_begin_contact;
    sp<Behavior::Method> _on_end_contact;
};

}

sp<CollisionCallback> CollisionCallbackType::create(Behavior& behavior)
{
    return create(behavior, "on_begin_contact", "on_end_contact");
}

sp<CollisionCallback> CollisionCallbackType::create(Behavior& behavior, StringView onBeginContact, StringView onEndContact)
{
    sp<Behavior::Method> onBeginContactMethod = behavior.getMethod(onBeginContact);
    sp<Behavior::Method> onEndContactMethod = behavior.getMethod(onEndContact);
    CHECK_WARN(onBeginContactMethod || onEndContactMethod, "Behavior has neither method defined(\"%s\", \"%s\")", onBeginContact.data(), onEndContact.data());
    return sp<CollisionCallback>::make<CollisionCallbackImpl>(std::move(onBeginContactMethod), std::move(onEndContactMethod));
}

}
