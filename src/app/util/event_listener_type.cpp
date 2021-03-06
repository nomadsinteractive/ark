#include "app/util/event_listener_type.h"

#include "core/epi/disposed.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"

#include "app/impl/event_listener/event_listener_list.h"

namespace ark {

sp<EventListener> EventListenerType::create(const sp<EventListener>& eventListener)
{
    return eventListener;
}

sp<EventListener> EventListenerType::makeDisposable(const sp<EventListener>& self, const sp<Boolean>& disposed)
{
    return self.absorb(disposed ? sp<Disposed>::make(disposed) : sp<Disposed>::make());
}

sp<EventListener> EventListenerType::makeAutoRelease(const sp<EventListener>& self, int32_t refCount)
{
    return makeDisposable(self, sp<BooleanByWeakRef<EventListener>>::make(self, refCount));
}

sp<EventListener> EventListenerType::BUILDER::build(const Scope& /*args*/)
{
    return sp<EventListenerList>::make();
}

}
