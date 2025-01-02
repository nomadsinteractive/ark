#pragma once

#include <set>

#include "core/base/api.h"
#include "core/types/box.h"
#include "core/types/class.h"
#include "core/types/type.h"

namespace ark {
namespace _internal {

template<typename T, typename... INTERFACES> void _add_types(std::set<TypeId>& interfaces) {
    interfaces.insert(Type<T>::id());
    if(const Class* clazz = Class::ensureClass<T>()) {
        interfaces.insert(clazz->id());
        interfaces.insert(clazz->implements().begin(), clazz->implements().end());
    }
    if constexpr(sizeof...(INTERFACES) > 0)
        _add_types<INTERFACES...>(interfaces);
}

template<typename... INTERFACES> std::set<TypeId> _make_types() {
    std::set<TypeId> interfaces;
    _add_types<INTERFACES...>(interfaces);
    return interfaces;
}

template<typename T, typename U = void, typename... INTERFACES> Box _dynamic_down_cast(const sp<T>& ptr, TypeId id) {
    if(Type<U>::id() == id) {
        sp<U> casted = ptr.template cast<U>();
        return Box(std::move(casted));
    }
    if(const Class* clazz = Class::ensureClass<U>(); clazz && clazz->isInstance(id))
        return clazz->cast(Box(ptr.template cast<U>()), id);
    if(sizeof...(INTERFACES) != 0)
       return _dynamic_down_cast<T, INTERFACES...>(ptr, id);
    return {};
}

template<typename T, typename U = void, typename... INTERFACES> Box _dynamic_up_cast(const Box& box) {
    if(Type<U>::id() == box.typeId()) {
        const sp<U>& unpacked = box.toPtr<U>();
        return Box(unpacked.template cast<T>());
    }
    if(sizeof...(INTERFACES) != 0)
       return _dynamic_up_cast<T, INTERFACES...>(box);
    return {};
}

template<typename T, typename... INTERFACES> class ClassImpl final : public IClass {
public:
    ClassImpl()
        : _id(Type<T>::id()) {
    }

    Box cast(const Box& box, TypeId id) override {
        if(box.typeId() == _id) {
            const sp<T>& ptr = box.toPtr<T>();
            return id == Type<T>::id() ? Box(ptr) : _dynamic_down_cast<T, INTERFACES...>(ptr, id);
        }
        const Box inst = _dynamic_up_cast<T, INTERFACES...>(box);
        if(id == Type<T>::id())
            return inst;
        return _dynamic_down_cast<T, INTERFACES...>(inst.toPtr<T>(), id);
    }

private:
    TypeId _id;
};

template<typename T, typename... INTERFACES> Class* _make_class() {
    Class* clazz = Class::addClass<T>("<Unknown>", std::make_unique<ClassImpl<T, INTERFACES...>>());
    clazz->setImplementation(_make_types<T, INTERFACES...>());
    return clazz;
}

}

template<typename T, typename... INTERFACES> class Implements {
protected:
    Implements() {
        [[maybe_unused]] static Class* clazz = _internal::_make_class<T, INTERFACES...>();
    }
};

}
