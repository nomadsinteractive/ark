#pragma once

#include <memory>
#include <map>
#include <set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/type.h"

namespace ark {

class ARK_API Interfaces {
public:
    Interfaces(Class* clazz);
    DISALLOW_COPY_AND_ASSIGN(Interfaces);

    template<typename T> bool is() const {
        return is(Type<T>::id());
    }

    template<typename T> bool isInstance() const {
        return isInstance(Type<T>::id());
    }

    template<typename T> SharedPtr<T> as(const Box& box) const {
        return as(box, Type<T>::id()).template unpack<T>();
    }

    TypeId typeId() const;
    const std::set<TypeId>& implements() const;
    Box as(const Box& box, TypeId id) const;
    bool is(TypeId id) const;
    bool isInstance(TypeId id) const;

private:
    Class* _class;
};

}
