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
        return _hash(__ARK_FUNCTION__);
    }

    static constexpr uint32_t _hash(const char* s) {
        return static_cast<uint32_t>(*s ? (*(s + 1) ? *s + (*(s + 1)) * 101ull +
                                                      (*(s + 2) ? (*(s + 3) ? *(s + 2) + (*(s + 3)) * 101ull + _hash(s + 4) * 101ull * 101ull : *(s + 2)) : 0)
                                                      * 101ull * 101ull : *s) : 0);
    }

private:
    enum : TypeId {
        TYPE_ID = _id()
    };
};

}
