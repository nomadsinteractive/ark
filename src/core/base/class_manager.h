#pragma once

#include <mutex>

#include "core/base/api.h"
#include "core/types/class.h"
#include "core/types/implements.h"

namespace ark {

class ARK_API ClassManager {
public:

    Class* addClass(TypeId id, const char* name, std::unique_ptr<IClass> impl);
    Class* ensureClass(TypeId id);

    static ClassManager& instance();

    template<typename T, typename... Args> void addClass(const char* name) {
        std::unique_ptr<Class> clazz(new Class(Type<T>::id(), name, std::make_unique<_internal::ClassImpl<T, Args...>>()));
        if constexpr(sizeof...(Args) > 0)
            clazz->setImplementation<Args...>();
        _classes[Type<T>::id()] = std::move(clazz);
    }

private:
    std::mutex _mutex;
    HashMap<TypeId, std::unique_ptr<Class>> _classes;
};

}
