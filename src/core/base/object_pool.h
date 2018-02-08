#ifndef ARK_CORE_TYPES_OBJECT_POOL_H_
#define ARK_CORE_TYPES_OBJECT_POOL_H_

#include <stdlib.h>
#include <unordered_map>

#include "core/base/api.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/types/shared_ptr.h"
#include "core/types/type.h"

namespace ark {

class ObjectPool {
private:
    struct Cached {
        Cached(size_t size)
            : _ptr(malloc(size)) {
        }
        ~Cached() {
            free(_ptr);
        }

        void* _ptr;
        std::shared_ptr<Interfaces> _interfaces;

        DISALLOW_COPY_AND_ASSIGN(Cached);
    };

public:
    ObjectPool()
        : _items(sp<std::unordered_map<TypeId, sp<LockFreeStack<sp<Cached>>>>>::make()) {
    }
    ObjectPool(const ObjectPool& other) = default;

    template<typename U, typename... Args> sp<U> obtain(Args&&... args) {
        const sp<LockFreeStack<sp<Cached>>>& queue = ensure(Type<U>::id());
        sp<Cached> cached;

        if(!queue->pop(cached))
            cached = sp<Cached>::make(sizeof(U));

        new(cached->_ptr) U(std::forward<Args>(args)...);

        if(cached->_interfaces)
            cached->_interfaces->reset();
        else
            cached->_interfaces = std::make_shared<Interfaces>(Class::getClass<U>());
        return SharedPtr<U>(reinterpret_cast<U*>(cached->_ptr), cached->_interfaces, [queue, cached] (U* obj) {
            obj->~U();
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

private:
    sp<std::unordered_map<TypeId, sp<LockFreeStack<sp<Cached>>>>> _items;
};

}

#endif
