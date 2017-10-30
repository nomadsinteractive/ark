#ifndef ARK_CORE_TYPES_TYPE_H_
#define ARK_CORE_TYPES_TYPE_H_

#include "core/base/api.h"

namespace ark {

template<typename T> class Type {
public:
    static _CONSTEXPR TypeId id() {
#ifdef ARK_USE_CONSTEXPR
        return TYPE_ID;
#else
        const static TypeId _ID = init();
        return _ID;
#endif
    }

private:
    static _CONSTEXPR uint32_t _hash(const char* s) {
        return static_cast<uint32_t>(*s ? (*(s + 1) ? *s + (*(s + 1)) * 101ull + _hash(s + 2) * 101ull * 101ull : *s) : 0);
    }

    static _CONSTEXPR TypeId init() {
        return _hash(__ARK_FUNCTION__);
    }

#ifdef ARK_USE_CONSTEXPR
private:
    enum : TypeId {
        TYPE_ID = init()
    };
#endif
};

}

#endif
