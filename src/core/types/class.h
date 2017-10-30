#ifndef ARK_CORE_TYPES_IMPLEMENTS_H_
#define ARK_CORE_TYPES_IMPLEMENTS_H_

#include <set>
#include <unordered_set>

#include "core/base/api.h"
#include "core/types/box.h"
#include "core/types/type.h"

namespace ark {

template<typename... Args> class Implementation {
public:
    template<typename T> static _CONSTEXPR bool is() {
        return check<T, Args...>();
    }

private:
    template<typename T> static _CONSTEXPR bool check() {
        return false;
    }

    template<typename T, typename U, typename... Types> static _CONSTEXPR bool check() {
        return std::is_same<T, U>::value ? true : (sizeof...(Types) == 0 ? sfinae_check<T, U>(true) : check<T, Types...>());
    }

    template<typename T, typename U> static _CONSTEXPR bool sfinae_check(const decltype(U::template is<T>())) {
        return U::template is<T>();
    }

    template<typename T, typename U> static _CONSTEXPR bool sfinae_check(...) {
        return std::is_same<T, U>::value;
    }
};

class ARK_API IClass {
public:
    virtual ~IClass() = default;

    virtual Box cast(const Box& box, TypeId id) = 0;
};

class ARK_API Class {
public:
    Class();
    Class(TypeId id);
    Class(TypeId id, const char* name, IClass* delegate);
    Class(const Class& other);
    Class(Class&& other);

    TypeId id() const;
    const char* name() const;
    const std::unordered_set<TypeId>& implements() const;

    bool isInstance(TypeId id) const;

    const Class& operator =(const Class& other);
    const Class& operator =(Class&& other);

    Box cast(const Box& box, TypeId id);

    void setImplementation(std::unordered_set<TypeId>&& implementation);

    template<typename T> static Class* getClass() {
        return getClass(Type<T>::id());
    }
    static Class* getClass(TypeId id);
    static Class* putClass(TypeId id, const char* name, IClass* impl);

private:
    template<typename T = void, typename... Args> void setImplementation() {
        TypeId id = Type<T>::id();
        if(id != Type<void>::id()) {
            _implements.insert(Type<T>::id());
            setImplementation<Args...>();
        }
    }

private:
    TypeId _id;
    const char* _name;
    std::unordered_set<TypeId> _implements;
    IClass* _delegate;

    friend class ClassManager;
};

namespace _internal {

template<typename T = void, typename... INTERFACES> void _add_types(std::unordered_set<TypeId>& interfaces) {
    interfaces.insert(Type<T>::id());
    Class* clazz = Class::getClass(Type<T>::id());
    if(clazz)
        interfaces.insert(clazz->implements().begin(), clazz->implements().end());
    if(sizeof...(INTERFACES) != 0)
        _add_types<INTERFACES...>(interfaces);
}

template<typename... INTERFACES> std::unordered_set<TypeId> _make_types() {
    std::unordered_set<TypeId> interfaces;
    _add_types<INTERFACES...>(interfaces);
    return interfaces;
}

template<typename T, typename U = void, typename... INTERFACES> Box _dynamic_down_cast(const sp<T>& ptr, TypeId id) {
    if(Type<U>::id() == id) {
        const sp<U>& casted = ptr.template cast<U>();
        return casted.pack();
    }
    Class* clazz = Class::getClass(Type<U>::id());
    if(clazz && clazz->isInstance(id))
        return clazz->cast(ptr.template cast<U>().pack(), id);
    if(sizeof...(INTERFACES) != 0)
       return _dynamic_down_cast<T, INTERFACES...>(ptr, id);
    return Box();
}

template<typename T, typename U = void, typename... INTERFACES> Box _dynamic_up_cast(const Box& box) {
    if(Type<U>::id() == box.typeId()) {
        const sp<U>& unpacked = box.unpack<U>();
        return unpacked.template cast<T>().pack();
    }
    if(sizeof...(INTERFACES) != 0)
       return _dynamic_up_cast<T, INTERFACES...>(box);
    return Box();
}

template<typename T, typename... INTERFACES> class _ClassImpl : public IClass {
public:
    _ClassImpl()
        : _id(Type<T>::id()) {
    }

    virtual Box cast(const Box& box, TypeId id) override {
        if(box.typeId() == _id) {
            const sp<T>& ptr = box.unpack<T>();
            return id == Type<T>::id() ? ptr.pack() : _dynamic_down_cast<T, INTERFACES...>(ptr, id);
        } else if(id == Type<T>::id()) {
            return _dynamic_up_cast<T, INTERFACES...>(box);
        }
        FATAL("Wrong type being casted: Cannot cast \"%s\" to \"%s\"", Class::getClass(_id)->name(), Class::getClass(id)->name());
        return Box();
    }

private:
    TypeId _id;
};

}

template<typename T, typename... INTERFACES> class Implements {
public:
    Implements() {
        static _internal::_ClassImpl<T, INTERFACES...> _CLASS_IMPL;
        Class* clazz = Class::putClass(Type<T>::id(), "<Unknown>", &_CLASS_IMPL);
        clazz->setImplementation(_internal::_make_types<T, INTERFACES...>());
    }
};

}

#endif
