#ifndef ARK_CORE_BASE_GENERIC_OBJECT_POOL_H_
#define ARK_CORE_BASE_GENERIC_OBJECT_POOL_H_

#include "core/base/object_pool.h"
#include "core/collection/by_type.h"

namespace ark {

class GenericObjectPool {
public:
    template<typename T> const sp<ObjectPool<T>>& getObjectPool() {
        const sp<ObjectPool<T>>& objectPool = _object_pools.get<ObjectPool<T>>();
        if(objectPool)
            return objectPool;
        _object_pools.put<ObjectPool<T>>(sp<ObjectPool<T>>::make());
        return _object_pools.get<ObjectPool<T>>();
    }

private:
    ByType _object_pools;
};

}

#endif
