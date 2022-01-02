#ifndef ARK_CORE_UTIL_OPERATORS_H_
#define ARK_CORE_UTIL_OPERATORS_H_

#include "core/util/math.h"

#include "core/impl/variable/variable_op2.h"

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

    template<typename T, typename P = T> class NumericFloorDiv {
    public:
        float operator()(T v1, P v2) {
            return std::floor(v1 / v2);
        }
    };

    template<typename T, typename P = T> class IntegerFloorDiv {
    public:
        int32_t operator()(T v1, P v2) {
            return static_cast<int32_t>(v1 / v2);
        }
    };

    template<typename T, typename P = T> class Dot {
    public:
        auto operator()(const T& v1, const P& v2)->decltype(v1.dot(v2)) {
            return v1.dot(v2);
        }
    };

    template<typename T, typename P = T> class Distance {
    public:
        float operator()(const T& v1, const P& v2) {
            return (v1 - v2).length();
        }
    };

    template<typename T, typename P = T> class Mod {
    public:
        auto operator()(T v1, P v2)->decltype(v1 / v2) {
            return Math::mod(v1, v2);
        }
    };

    template<typename T, typename P = T> class ModFloor {
    public:
        auto operator()(T v1, P v2)->decltype(v1 / v2) {
            return Math::modFloor(v1, v2);
        }
    };

    template<typename T, typename P = T> class Pow {
    public:
        float operator()(T v1, P v2) {
            return static_cast<float>(std::pow(v1, v2));
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

    template<typename T> class Not {
    public:
        T operator()(T val) {
            return !val;
        }
    };

    template<typename T> class Min {
    public:
        T operator()(T v1, T v2) {
            return v1 < v2 ? v1 : v2;
        }
    };

    template<typename T> class Max {
    public:
        T operator()(T v1, T v2) {
            return v1 > v2 ? v1 : v2;
        }
    };

    template<typename T, typename P> class Cast {
    public:
        P operator()(T v1) {
            return static_cast<P>(v1);
        }
    };

    class Atan2 {
    public:
        float operator()(float v1, float v2) {
            return Math::atan2(v1, v2);
        }
    };

    template<typename T, typename OP> class Builder : public ark::Builder<Variable<T>> {
    public:
        typedef sp<ark::Builder<Variable<T>>> BuilderType;

        Builder(const BuilderType& a1, const BuilderType& a2)
            : _a1(a1), _a2(a2) {
        }

        virtual sp<Variable<T>> build(const Scope& args) override {
            return sp<VariableOP2<sp<Variable<T>>, sp<Variable<T>>, OP>>::make(_a1->build(args), _a2->build(args));
        }

    private:
        BuilderType _a1;
        BuilderType _a2;
    };
};

}

#endif
