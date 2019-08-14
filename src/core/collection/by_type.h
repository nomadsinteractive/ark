#ifndef ARK_CORE_COLLECTION_BY_TYPE_H_
#define ARK_CORE_COLLECTION_BY_TYPE_H_

#include <unordered_map>

#include "core/base/api.h"
#include "core/types/box.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"
#include "core/types/type.h"

namespace ark {

class ARK_API ByType {
public:
    ByType() = default;
    DEFAULT_COPY_AND_ASSIGN(ByType);

    template<typename T> bool has() {
        return _items.find(Type<T>::id()) != _items.end();
    }

    template<typename T> const Box& put(const sp<T>& item) {
        Box& slot = _items[Type<T>::id()];
        slot = item;
        return slot;
    }

    template<typename T> const sp<T>& get() const {
        const auto iter = _items.find(Type<T>::id());
        return iter != _items.end() ? iter->second.template unpack<T>() : sp<T>::null();
    }

    template<typename T, typename... Args> const sp<T>& ensure(Args&&... args) {
        return instance_sfinae<T, Args...>(0, std::forward<Args>(args)...);
    }

private:
    template<typename T, typename... Args> const sp<T>& instance_sfinae(typename std::enable_if<std::is_constructible<T, Args...>::value, int32_t>::type, Args&&... args) {
        const sp<T>& inst = get<T>();
        if(inst)
            return inst;
        return put<T>(sp<T>::make(std::forward<Args>(args)...)).template unpack<T>();
    }

    template<typename T> const sp<T>& instance_sfinae(...) {
        const sp<T>& inst = get<T>();
        DCHECK(inst, "Cannot get instance and there is no way to build one(it is abstract or has no default constructor)");
        return inst;
    }

private:
    std::unordered_map<TypeId, Box> _items;
};

}

#endif
