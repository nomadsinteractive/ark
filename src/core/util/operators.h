#ifndef ARK_CORE_UTIL_OPERATORS_H_
#define ARK_CORE_UTIL_OPERATORS_H_

#include "core/util/math.h"

namespace ark {

class Operators {
public:
    template<typename T> class GT {
    public:
        bool operator()(T v1, T v2) {
            return v1 > v2;
        }
    };

    template<typename T> class GE {
    public:
        bool operator()(T v1, T v2) {
            return v1 >= v2;
        }
    };

    template<typename T> class LT {
    public:
        bool operator()(T v1, T v2) {
            return v1 < v2;
        }
    };

    template<typename T> class LE {
    public:
        bool operator()(T v1, T v2) {
            return v1 <= v2;
        }
    };

    template<typename T> class EQ {
    public:
        bool operator()(T v1, T v2) {
            return v1 == v2;
        }
    };

    template<typename T> class NE {
    public:
        bool operator()(T v1, T v2) {
            return v1 != v2;
        }
    };

    template<typename T> class Add {
    public:
        T operator()(T v1, T v2) {
            return v1 + v2;
        }
    };

    template<typename T> class Sub {
    public:
        T operator()(T v1, T v2) {
            return v1 - v2;
        }
    };

    template<typename T, typename P = T> class Mul {
    public:
        auto operator()(T v1, P v2)->decltype(v1 * v2) {
            return v1 * v2;
        }
    };

    template<typename T, typename P = T> class Div {
    public:
        auto operator()(T v1, P v2)->decltype(v1 / v2) {
            return v1 / v2;
        }
    };

    template<typename T, typename P = T> class Mod {
    public:
        auto operator()(T v1, P v2)->decltype(v1 / v2) {
            return Math::mod(v1, v2);
        }
    };

    template<typename T> class And {
    public:
        auto operator()(T v1, T v2)->decltype(v1 && v2) {
            return v1 && v2;
        }
    };

    template<typename T> class Or {
    public:
        auto operator()(T v1, T v2)->decltype(v1 || v2) {
            return v1 || v2;
        }
    };
};

}

#endif
