#pragma once

#include <mutex>
#include <unordered_map>

#include "core/base/api.h"
#include "core/types/class.h"
#include "core/types/implements.h"

namespace ark {

class ARK_API ClassManager {
public:

    Class* addClass(TypeId id, const char* name, std::unique_ptr<IClass> impl);

    template<typename T, typename... Args> void addClass(const char* name) {
        Class* clazz = new Class(Type<T>::id(), name, std::make_unique<_internal::ClassImpl<T, Args...>>());
        if constexpr(sizeof...(Args) > 0)
            clazz->setImplementation<Args...>();
        _classes[Type<T>::id()] = std::unique_ptr<Class>(clazz);
    }

    Class* obtain(TypeId id);

    void updateHierarchy();

    static ClassManager& instance();

private:
    std::mutex _mutex;
    std::unordered_map<TypeId, std::unique_ptr<Class>> _classes;
};

}
