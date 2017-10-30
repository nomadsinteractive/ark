#ifndef ARK_CORE_BASE_LIBRARY_H_
#define ARK_CORE_BASE_LIBRARY_H_

#include <functional>

#include "core/base/callable.h"
#include "core/collection/by_name.h"
#include "core/forwarding.h"
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
    Library() {
    }
    Library(const Library& other)
        : _callables(other._callables) {
    }
    Library(Library&& other)
        : _callables(std::move(other._callables)){
    }

    const Library& operator = (const Library& other) {
        _callables = other._callables;
        return *this;
    }

    const Library& operator = (Library&& other) {
        _callables = std::move(other._callables);
        return *this;
    }

    template<typename T> const sp<Callable<T>>& getCallable(const String& name) const {
        Function<T> func;
        return _callables.get<Callable<T>>(func.name(name));
    }

    template<typename T> void addCallable(const String& name, const std::function<T>& func) {
        addCallable(name, sp<Callable<T>>::make(func));
    }

    template<typename T> void addCallable(const String& name, const sp<Callable<T>>& callable) {
        Function<T> func;
        _callables.put<Callable<T>>(func.name(name), callable);
    }

private:
    ByName _callables;
};

}

#endif
