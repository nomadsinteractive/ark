#pragma once

#include "core/base/api.h"

namespace ark {

template<typename T> class Type {
public:
    static constexpr TypeId id() {
        return TYPE_ID;
    }

private:
    static constexpr TypeId _id() {
        return string_hash(__ARK_FUNCTION__);
    }

private:
    enum : TypeId {
        TYPE_ID = _id()
    };
};

}
