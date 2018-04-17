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

    template<typename T> void put(const sp<T>& item) {
        _items[Type<T>::id()] = item.pack();
    }

    template<typename T> const sp<T>& get() const {
        const auto iter = _items.find(Type<T>::id());
        return iter != _items.end() ? iter->second.template unpack<T>() : sp<T>::null();
    }

    template<typename T> const sp<T>& ensure() {
        return instance_sfinae<T>(0);
    }

private:
    template<typename T> const sp<T>& instance_sfinae(typename std::enable_if<!std::is_abstract<T>::value && std::is_constructible<T>::value, int>::type) {
        const sp<T>& inst = get<T>();
        if(inst)
            return inst;
        put<T>(sp<T>::make());
        return get<T>();
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
