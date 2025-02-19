#pragma once

#include <any>
#include <functional>

#include "core/forwarding.h"
#include "core/types/optional.h"
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

    template<typename T> Optional<std::function<T>> getCallable(const String& name) const {
        Function<T> func;
        const auto iter = _callables.find(func.name(name));
        return iter != _callables.end() ? Optional<std::function<T>>(std::any_cast<std::function<T>>(iter->second)) : Optional<std::function<T>>();
    }

    template<typename T> void addCallable(const String& name, std::function<T> callable) {
        Function<T> func;
        _callables.insert(std::make_pair(func.name(name), std::make_any<std::function<T>>(std::move(callable))));
    }

private:
    Map<String, std::any> _callables;
};

}
