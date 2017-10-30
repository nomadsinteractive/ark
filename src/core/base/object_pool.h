#ifndef ARK_CORE_TYPES_OBJECT_POOL_H_
#define ARK_CORE_TYPES_OBJECT_POOL_H_

#include <stdlib.h>
#include <unordered_map>

#include "core/base/api.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/types/shared_ptr.h"
#include "core/types/type.h"

namespace ark {

template<typename T> class ObjectPool {
private:
    class Cached {
    public:
        Cached(size_t size)
            : _ptr(reinterpret_cast<T*>(malloc(size))), _interfaces(std::make_shared<Interfaces>()) {
        }
        Cached(const Cached& other) = delete;

        ~Cached() {
            free(_ptr);
        }

        T* ptr() const {
            return _ptr;
        }

        const std::shared_ptr<Interfaces>& interfaces() const {
            return _interfaces;
        }

    private:
        T* _ptr;
        std::shared_ptr<Interfaces> _interfaces;
    };

public:
    ObjectPool()
        : _items(sp<std::unordered_map<TypeId, sp<LockFreeStack<sp<Cached>>>>>::make()) {
    }

    template<typename U, typename... Args> sp<T> allocate(Args... args) {
        const sp<LockFreeStack<sp<Cached>>>& queue = ensure(Type<U>::id());
        sp<Cached> cached;

        if(!queue->pop(cached))
            cached = sp<Cached>::make(sizeof(U));

        new(cached->ptr()) U(std::forward<Args>(args)...);

        return SharedPtr<T>(cached->ptr(), cached->interfaces(), [queue, cached] (T* obj) {
            obj->~T();
            queue->push(cached);
        });
    }

private:
    const sp<LockFreeStack<sp<Cached>>>& ensure(TypeId typeId) {
        auto iter = _items->find(typeId);
        if(iter != _items->end())
            return iter->second;
        (*_items)[typeId] = sp<LockFreeStack<sp<Cached>>>::make();
        return (*_items)[typeId];
    }

    sp<std::unordered_map<TypeId, sp<LockFreeStack<sp<Cached>>>>> _items;
};

}

#endif
