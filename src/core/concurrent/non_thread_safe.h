#pragma once

#include <thread>

#include "core/base/api.h"

namespace ark {

template<THREAD_NAME_ID TID = THREAD_NAME_ID_UNSPECIFIED> class NonThreadSafe {
public:
#if ARK_FLAG_DEBUG
    class Synchronized {
    public:
        ~Synchronized() {
            _owner._synchronized_thread_id = {};
        }

    private:
        Synchronized(const NonThreadSafe& owner)
            : _owner(owner) {
            DASSERT(_owner._synchronized_thread_id == std::thread::id());
            _owner._synchronized_thread_id = std::this_thread::get_id();
        }

    private:
        const NonThreadSafe& _owner;
        friend class NonThreadSafe;
    };
#else
    typedef int32_t Synchronized;
#endif

    NonThreadSafe()
#if ARK_FLAG_DEBUG
        : _owner_thread_id(std::this_thread::get_id())
#endif
    {
    }

[[nodiscard]]
    Synchronized synchronize() const {
#if ARK_FLAG_DEBUG
        return {*this};
#else
        return 0;
#endif
    }

    void safetyCheck() const {
#if ARK_FLAG_DEBUG
        if constexpr(TID != THREAD_NAME_ID_UNSPECIFIED)
            if(__thread_check__(TID))
                return;
        const std::thread::id threadId = std::this_thread::get_id();
        DCHECK(threadId == _owner_thread_id || threadId == _synchronized_thread_id, "Thread safety policy violation");
#endif
    }

private:
#if ARK_FLAG_DEBUG
    std::thread::id _owner_thread_id;
    mutable std::thread::id _synchronized_thread_id;
#endif
};

}