#ifndef ARK_CORE_BASE_LIBRARY_H_
#define ARK_CORE_BASE_LIBRARY_H_

#include <functional>
#include <unordered_map>

#include "core/base/callable.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"

namespace ark {

class Library {
private:
    template<typename T> class Function {
    public:
        String name(const String& name) const {
            return name;
        }
    };

    template<typename R, typename... Args> class Function<R(Args...)> {
    public:
        String name(const String& name) const {
            return Strings::sprintf("%s$%d", name.c_str(), sizeof...(Args));
        }
    };

public:
    Library() = default;
    DEFAULT_COPY_AND_ASSIGN(Library);

    template<typename T> const sp<Callable<T>>& getCallable(const String& name) const {
        Function<T> func;
        const auto iter = _callables.find(func.name(name));
        return iter != _callables.end() ? iter->second.unpack<Callable<T>>() : sp<Callable<T>>::null();
    }

    template<typename T> void addCallable(const String& name, const std::function<T>& func) {
        addCallable(name, sp<Callable<T>>::make(func));
    }

    template<typename T> void addCallable(const String& name, const sp<Callable<T>>& callable) {
        Function<T> func;
        _callables.insert(std::make_pair(func.name(name), callable.pack()));
    }

private:
    std::unordered_map<String, Box> _callables;
};

}

#endif
