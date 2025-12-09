#include "app/components/behavior.h"

#include "core/ark.h"
#include "core/inf/interpreter.h"
#include "core/util/log.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/base/event.h"
#include "app/base/searching_node.h"
#include "app/components/rigidbody.h"
#include "app/inf/application_event_listener.h"
#include "app/inf/collision_callback.h"
#include "app/inf/searching_node_provider.h"

namespace ark {

namespace {

class RunnableImpl final : public Runnable, public Debris {
public:
    RunnableImpl(sp<Interpreter> interpreter, Box function)
        : _interpreter(std::move(interpreter)), _function(std::move(function))
    {
    }

    void run() override
    {
        _interpreter->call(_function, {});
    }

    void traverse(const Visitor& visitor) override
    {
        visitor(_function);
    }

private:
    sp<Interpreter> _interpreter;
    Box _function;
};

class CollisionCallbackImpl final : public CollisionCallback {
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

class EventListenerImpl final : public EventListener {
public:
    EventListenerImpl(sp<Behavior::Method> onEvent)
        : _on_event(std::move(onEvent))
    {
    }

    bool onEvent(const Event& event) override
    {
        Box arg1(sp<Event>::make(event));
        return static_cast<bool>(_on_event->call({std::move(arg1)}));
    }

private:
    sp<Behavior::Method> _on_event;
};

class SearchingNodeProviderImpl final : public SearchingNodeProvider {
public:
    SearchingNodeProviderImpl(sp<Behavior::Method> method)
        : _method(std::move(method)) {
    }

    void onVisitAdjacentNodes(const V3& position, const std::function<void(SearchingNode, float)>& visitor) override
    {
        auto v = [this, &position, &visitor](const V3& pos) {
            const float weight = Math::distance(position, pos);
            visitor(SearchingNode(*this, pos), weight);
        };

        _method->call({
            Box(sp<Vec3>::make<Vec3::Const>(position)),
            Box(sp<std::function<void(const V3&)>>::make(std::move(v)))
        });
    }

private:
    sp<Behavior::Method> _method;
};

class ApplicationEventListenerImpl final : public ApplicationEventListener {
public:
    ApplicationEventListenerImpl(sp<Behavior::Method> onPause, sp<Behavior::Method> onResume, sp<Behavior::Method> onUnhandledEvent)
        : _on_pause(std::move(onPause)), _on_resume(std::move(onResume)), _on_unhandled_event(std::move(onUnhandledEvent))
    {
    }

    void onPause() override
    {
        _on_pause->call({});
    }

    void onResume() override
    {
        _on_resume->call({});
    }

    bool onUnhandledEvent(const Event& event) override
    {
        Box arg1(sp<Event>::make(event));
        return static_cast<bool>(_on_unhandled_event->call({std::move(arg1)}));
    }

private:
    sp<Behavior::Method> _on_pause;
    sp<Behavior::Method> _on_resume;
    sp<Behavior::Method> _on_unhandled_event;
};

}

Behavior::Behavior(Box delegate)
    : _interpreter(Ark::instance().applicationContext()->interpreter()), _delegate(std::move(delegate))
{
}

void Behavior::onWire(const WiringContext& context, const Box& self)
{
    _with_debris.onWire(context, self);
}

void Behavior::traverse(const Visitor& visitor)
{
    visitor(_delegate);
    _with_debris.traverse(visitor);
}

sp<Runnable> Behavior::subscribe(const StringView name)
{
    Box function = _interpreter->attr(_delegate, name);
    CHECK(function, "Object has no attribute \"%s\"", name.data());
    return doCreateRunnable(std::move(function));
}

sp<Runnable> Behavior::createRunnable(const StringView name)
{
    Box function = _interpreter->attr(_delegate, name);
    CHECK(function, "Object has no attribute \"%s\"", name.data());
    return doCreateRunnable(std::move(function));
}

sp<CollisionCallback> Behavior::createCollisionCallback(const StringView onBeginContact, const StringView onEndContact)
{
    sp<Method> onBeginContactMethod = getMethod(onBeginContact);
    sp<Method> onEndContactMethod = getMethod(onEndContact);
    CHECK(onBeginContactMethod || onEndContactMethod, "Behavior has neither method defined(\"%s\", \"%s\")", onBeginContact.data(), onEndContact.data());
    return sp<CollisionCallback>::make<CollisionCallbackImpl>(std::move(onBeginContactMethod), std::move(onEndContactMethod));
}

sp<EventListener> Behavior::createEventListener(const StringView onEvent)
{
    return sp<EventListener>::make<EventListenerImpl>(getMethod(onEvent));
}

sp<SearchingNodeProvider> Behavior::createSearchingNodeProvider(const StringView onVisitAdjacentNodes)
{
    return sp<SearchingNodeProvider>::make<SearchingNodeProviderImpl>(getMethod(onVisitAdjacentNodes));
}

sp<ApplicationEventListener> Behavior::createApplicationEventListener(const StringView onPause, const StringView onResume, const StringView onUnhandledEvent)
{
    return sp<ApplicationEventListener>::make<ApplicationEventListenerImpl>(getMethod(onPause), getMethod(onResume), getMethod(onUnhandledEvent));
}

sp<Runnable> Behavior::doCreateRunnable(Box function)
{
    sp<RunnableImpl> runnable = sp<RunnableImpl>::make(_interpreter, std::move(function));
    _with_debris.track(runnable);
    return runnable;
}

Behavior::Method::Method(sp<Interpreter> interpreter, Box function)
    : _interpreter(std::move(interpreter)), _function(std::move(function))
{
}

Box Behavior::Method::call(const Interpreter::Arguments& args) const
{
    return _interpreter->call(_function, args);
}

void Behavior::Method::traverse(const Visitor& visitor)
{
    visitor(_function);
}

sp<Behavior::Method> Behavior::getMethod(const StringView name)
{
    Box function = _interpreter->attr(_delegate, name);
    CHECK(function, "Object has no attribute \"%s\"", name.data());
    sp<Method> method = sp<Method>::make(_interpreter, function);
    _with_debris.track(method);
    return method;
}

}
