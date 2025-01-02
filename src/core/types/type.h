#pragma once

#include "core/forwarding.h"

namespace ark {

constexpr TypeId toVectorTypeId(const TypeId typeId) {
    return string_hash("Vector<>", typeId);
}

template<typename T> class Type {
public:
    static constexpr TypeId id() {
        return string_hash(__ARK_FUNCTION__);
    }

    static constexpr TypeId vectorId() {
        return toVectorTypeId(id());
    }
};

template<typename T> class Type<Vector<T>> {
public:
    static constexpr TypeId id() {
        return Type<T>::vectorId();
    }

    static constexpr TypeId vectorId() {
        return toVectorTypeId(id());
    }
};

}
