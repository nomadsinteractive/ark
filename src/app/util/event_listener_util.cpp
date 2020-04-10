#include "app/util/event_listener_util.h"

#include "core/epi/disposed.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"

namespace ark {

sp<EventListener> EventListenerUtil::create(const sp<EventListener>& eventListener)
{
    return eventListener;
}

sp<EventListener> EventListenerUtil::makeDisposable(const sp<EventListener>& self, const sp<Boolean>& disposed)
{
    return self.absorb(disposed ? sp<Disposed>::make(disposed) : sp<Disposed>::make());
}

sp<EventListener> EventListenerUtil::makeAutoRelease(const sp<EventListener>& self, int32_t refCount)
{
    return makeDisposable(self, sp<BooleanByWeakRef<EventListener>>::make(self, refCount));
}

}
