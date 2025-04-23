#pragma once

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

    template<typename T> class AlmostEQ {
    public:
        AlmostEQ(const int32_t ulp)
            : _ulp(ulp) {
        }

        bool operator()(T v1, T v2) {
            return Math::almostEqual(v1, v2, _ulp);
        }

    private:
        int32_t _ulp;
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

    template<typename T, typename P = T> class FloorDiv {
    public:
        T operator()(T v1, P v2) {
            return Math::floorDiv(v1, v2);
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
            return (v1 - v2).hypot();
        }
    };

    template<typename T, typename P = T> class Distance2 {
    public:
        float operator()(const T& v1, const P& v2) {
            const auto d = v1 - v2;
            return d.dot(d);
        }
    };

    template<typename T> class Hypot {
    public:
        float operator()(const T& v) {
            return v.hypot();
        }
    };

    template<typename T, typename P = T> class Mod {
    public:
        auto operator()(T v1, P v2)->decltype(v1 / v2) {
            return Math::mod(v1, v2);
        }
    };

    template<typename T> class Floor {
    public:
        T operator()(T v) {
            return std::floor(v);
        }
    };

    template<typename T> class Ceil {
    public:
        T operator()(T v) {
            return std::ceil(v);
        }
    };

    template<typename T, typename UO, size_t N> class Transform {
    public:
        Transform(UO unaryOp)
            : _unary_op(std::move(unaryOp)) {
        }

        T operator()(const T& v) {
            T result;
            for(size_t i = 0; i < N; ++i)
                result[i] = _unary_op(v[i]);
            return result;
        }

    private:
        UO _unary_op;
    };

    template<typename T> class ModFloor {
    public:
        T operator()(const T& v1, const T& v2) {
            return Math::modFloor(v1, v2);
        }
    };

    template<typename T> class ModCeil {
    public:
        T operator()(const T& v1, const T& v2) {
            return Math::modFloor(v1, v2) + v2;
        }
    };

    template<typename T> class Round {
    public:
        T operator()(T v1) {
            return Math::round(v1);
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

    template<typename T> class Neg {
    public:
        T operator()(const T& val) {
            return -val;
        }
    };

    template<typename T> class Abs {
    public:
        T operator()(const T& val) {
            return Math::abs(val);
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

    template<typename T, typename U> class Extend {
    public:
        auto operator()(const T& v1, const U& v2) -> decltype(v1.extend(v2)) {
            return v1.extend(v2);
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

    template<typename T, typename U> class Subscript {
    public:
        Subscript(int32_t idx)
            : _idx(idx) {
        }

        U operator()(const T& val) {
            return static_cast<U>(val[_idx]);
        }

    private:
        int32_t _idx;
    };

    template<typename T> class Normalize {
    public:
        T operator()(const T& v) {
            return Math::normalize(v);
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
