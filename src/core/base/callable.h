#pragma once

#include <functional>
#include <type_traits>

#include "core/forwarding.h"

namespace ark {

template<typename T> class Callable;

template<typename R, typename... Args> class Callable<R(Args...)> {
public:
    Callable(std::function<R(Args...)> callable)
        : _callable(std::move(callable)) {
    }
    DEFAULT_COPY_AND_ASSIGN(Callable);

    R call(Args&&... args) const {
        return _callable(std::forward<Args>(args)...);
    }

private:
    std::function<R(Args...)> _callable;
};

}
