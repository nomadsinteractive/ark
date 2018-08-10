#ifndef ARK_CORE_UTIL_OPERATORS_H_
#define ARK_CORE_UTIL_OPERATORS_H_

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
            return v1 == v2;
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

    template<typename T> class Mul {
    public:
        T operator()(T v1, T v2) {
            return v1 * v2;
        }
    };

    template<typename T> class Div {
    public:
        T operator()(T v1, T v2) {
            return v1 / v2;
        }
    };
};

}

#endif
