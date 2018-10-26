#ifndef ARK_CORE_BASE_CALLABLE_H_
#define ARK_CORE_BASE_CALLABLE_H_

#include <functional>
#include <type_traits>

#include "core/forwarding.h"
#include "core/util/conversions.h"

namespace ark {

template<typename T> class Callable;

template<typename R, typename... Args> class Callable<R(Args...)> {
public:
    Callable(const std::function<R(Args...)>& callable)
        : _callable(callable) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Callable);

    template<typename... Params> R call(Params&&... params) const {
        return _callable(std::forward<Params>(params)...);
    }

private:
    std::function<R(Args...)> _callable;

};

}

#endif
