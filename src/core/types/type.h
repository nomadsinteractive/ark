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
        const static TypeId _ID = _id();
        return _ID;
#endif
    }

private:
    static _CONSTEXPR TypeId _id() {
        return _hash(__ARK_FUNCTION__);
    }

    static _CONSTEXPR uint32_t _hash(const char* s) {
        return static_cast<uint32_t>(*s ? (*(s + 1) ? *s + (*(s + 1)) * 101ull +
                                                      (*(s + 2) ? (*(s + 3) ? *(s + 2) + (*(s + 3)) * 101ull + _hash(s + 4) * 101ull * 101ull : *(s + 2)) : 0)
                                                      * 101ull * 101ull : *s) : 0);
    }

#ifdef ARK_USE_CONSTEXPR
private:
    enum : TypeId {
        TYPE_ID = _id()
    };
#endif
};

}

#endif
