#pragma once

#include <mutex>
#include <thread>

#include "core/base/api.h"

namespace ark {

template<bool ENABLED> class NonThreadSafe {
public:
    class Synchronized {
    public:
        ~Synchronized() {
            _owner._synchronized_thread_id = {};
        }
        DISALLOW_COPY_AND_ASSIGN(Synchronized);

    private:
        Synchronized();
        Synchronized(const NonThreadSafe& owner, std::mutex& mutex)
            : _owner(owner), _lock_guard(mutex) {
            DASSERT(_owner._synchronized_thread_id == std::thread::id());
            _owner._synchronized_thread_id = std::this_thread::get_id();
        }

    private:
        const NonThreadSafe& _owner;
        std::lock_guard<std::mutex> _lock_guard;

        friend class NonThreadSafe;
    };

    NonThreadSafe() = default;

[[nodiscard]]
    Synchronized threadSynchronize(std::mutex& mutex) const {
        if constexpr(ENABLED)
            return {*this, mutex};
        else
            return {};
    }

    void threadCheck() const {
#if ARK_FLAG_BUILD_TYPE
        if constexpr(ENABLED) {
            // Most of the instances should be safely accessed on core thread.
            if(__thread_check__(THREAD_NAME_ID_CORE))
                return;
            const std::thread::id threadId = std::this_thread::get_id();
            if(_owner_thread_id == std::thread::id())
                _owner_thread_id = threadId;
            DCHECK(threadId == _owner_thread_id || threadId == _synchronized_thread_id, "Thread safety policy violation");
        }
#endif
    }

private:
#if ARK_FLAG_BUILD_TYPE
    mutable std::thread::id _owner_thread_id;
    mutable std::thread::id _synchronized_thread_id;
#endif
};

}