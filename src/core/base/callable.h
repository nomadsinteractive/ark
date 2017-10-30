#ifndef ARK_CORE_BASE_CALLABLE_H_
#define ARK_CORE_BASE_CALLABLE_H_

#include <functional>
#include <type_traits>

#include "core/util/conversions.h"

namespace ark {

template<typename T> class Callable;

template<typename R, typename... Args> class Callable<R(Args...)> {
public:
    Callable(const std::function<R(Args...)>& callable)
        : _callable(callable) {
    }
    Callable(std::function<R(Args...)>&& callable)
        : _callable(std::move(callable)) {
    }
    Callable(const Callable& other)
        : _callable(other._callable) {
    }
    Callable(Callable&& other)
        : _callable(std::move(other._callable)) {
    }

    const Callable<R(Args...)>& operator =(const Callable<R(Args...)>& other) {
        _callable = other._callable;
        return *this;
    }

    const Callable<R(Args...)>& operator =(Callable<R(Args...)>&& other) {
        _callable = std::move(other._callable);
        return *this;
    }

    template<typename... Params> R call(Params&&... params) const {
//        const std::tuple<Params...> args(std::forward<Params>(params)...);
//        return apply(args);
        return _callable(std::forward<Params>(params)...);
    }

private:
/*
    template<typename... Params> R apply(const std::tuple<Params...>& params) const {
        const std::size_t argN = sizeof...(Params);
        return _apply<std::tuple<Params...>, argN>(params);
    }

    template<typename T, std::size_t ArgN, std::size_t... Indices> R _apply(const T& params) const {
        const std::size_t indicesLen = sizeof...(Indices);
        if(ArgN == indicesLen) {
            std::tuple<typename std::remove_const<typename std::remove_reference<Args>::type>::type...> args;
            if(sizeof...(Indices) > 0)
                _tuple_decay<T, decltype(args), (sizeof...(Indices) > 1), Indices...>(params, args);
            return _call_sfinae<ArgN == indicesLen, decltype(args), Indices...>(args, nullptr);
        }
        const bool recurision = ArgN > indicesLen;
        return _apply_sfinae<recurision, T, ArgN, Indices..., indicesLen>(params, nullptr);
    }

    template<typename F, typename T, bool recurision, std::size_t I = 0, std::size_t... Indices> void _tuple_decay(const F& ft, T& tt) const {
        typedef typename std::tuple_element<I, F>::type FromType;
        typedef typename std::tuple_element<I, T>::type ToType;
        const FromType& from = std::get<I>(ft);
        std::get<I>(tt) = _decay_sfinae<FromType, ToType, !std::is_constructible<ToType, const FromType&>::value>(from, nullptr);
        if(recurision)
            _tuple_decay<F, T, (sizeof...(Indices) > 0), Indices...>(ft, tt);
    }

    template<typename F, typename T, bool convert> T _decay_sfinae(const F& from, typename std::enable_if<convert>::type*) const {
        return Conversions::to<F, T>(from);
    }

    template<typename F, typename T, bool convert> T _decay_sfinae(const F& from, ...) const {
        return from;
    }

    template<bool enabled, typename T, std::size_t ArgN, std::size_t... Indices> R _apply_sfinae(const T& params, typename std::enable_if<enabled>::type*) const {
        return _apply<T, ArgN, Indices...>(params);
    }

    template<bool enabled, typename T, std::size_t ArgN, std::size_t... Indices> R _apply_sfinae(const T& , ...) const {
        return _call_return<std::is_arithmetic<R>::value, R>(nullptr);
    }

    template<bool enabled, typename T, std::size_t... Indices> R _call_sfinae(const T& params, typename std::enable_if<enabled>::type*) const {
        return _callable(std::get<Indices>(params)...);
    }

    template<bool enabled, typename T, std::size_t... Indices> R _call_sfinae(const T& , ...) const {
        return _call_return<std::is_arithmetic<R>::value, R>(nullptr);
    }

    template<bool enabled, typename T> T _call_return(typename std::enable_if<enabled>::type*) const {
        return static_cast<T>(0);
    }

    template<bool enabled, typename T> T _call_return(...) const {
        return static_cast<T>(nullptr);
    }
*/
private:
    std::function<R(Args...)> _callable;

};

}

#endif
