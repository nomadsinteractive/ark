#ifndef ARK_CORE_BASE_CLASS_MANAGER_H_
#define ARK_CORE_BASE_CLASS_MANAGER_H_

#include <mutex>
#include <unordered_map>

#include "core/base/api.h"
#include "core/types/class.h"

namespace ark {

class ARK_API ClassManager {
public:

    Class* addClass(TypeId id, const char* name, IClass* impl);

    template<typename T, typename... Args> void addClass(const char* name) {
        static _internal::_ClassImpl<T, Args...> delegate;
        Class clazz(Type<T>::id(), name, &delegate);
        clazz.setImplementation<Args...>();
        _classes[Type<T>::id()] = clazz;
    }

    Class* obtain(TypeId id);

    void updateHierarchy();

    static ClassManager& instance();

private:
    std::mutex _mutex;
    std::unordered_map<TypeId, Class> _classes;
};

}

#endif
