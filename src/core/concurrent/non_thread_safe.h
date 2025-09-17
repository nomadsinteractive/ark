#pragma once

#include <thread>

#include "core/base/api.h"

namespace ark {

class NonThreadSafe {
public:
    class Synchronized {
    public:
        ~Synchronized() {
            _owner._synchronized_thread_id = {};
        }
        DISALLOW_COPY_AND_ASSIGN(Synchronized);

    private:
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

    NonThreadSafe()
#if ARK_FLAG_DEBUG
        : _owner_thread_id(std::this_thread::get_id())
#endif
    {
    }

[[nodiscard]]
    Synchronized threadSynchronize(std::mutex& mutex) const {
        return {*this, mutex};
    }

    void threadCheck() const {
#if ARK_FLAG_DEBUG
        // Most of the instances should be safely accessed on core thread.
        if(__thread_check__(THREAD_NAME_ID_CORE))
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