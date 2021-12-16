#ifndef ARK_CORE_UTIL_MATH_H_
#define ARK_CORE_UTIL_MATH_H_

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class Math {
public:

    template<typename T> static bool between(T a1, T a2, T val) {
        return a1 <= a2 ? (a1 <= val && val <= a2) : (a2 <= val && val <= a1);
    }

    template<typename T> static T divmod(T value, T mod, T& remainder) {
        T quot = static_cast<int32_t>(value /  mod);
        remainder = value - quot * mod;
        return quot;
    }

    template<typename T> static T mod(T value, T mod) {
        T v = value - static_cast<int32_t>(value /  mod) * T(mod);
        return signEquals(v, mod) ? v : v + mod;
    }

    template<typename T> static T modFloor(T value, T mod) {
        return value - Math::mod<T>(value, mod);
    }

    template<typename T> static T modCeil(T value, T mod) {
        return modFloor<T>(value, mod) + mod;
    }

    template<typename T> static void modBetween(T start, T end, T mod, T& modFloor, T& modCelling) {
        modFloor = Math::modFloor<T>(start, mod);
        modCelling = Math::modCeil<T>(end, mod);
    }

    template<typename T> static bool signEquals(T v1, T v2) {
        if((v1 <= 0 && v2 <= 0) || (v1 >= 0 && v2 >= 0))
            return true;
        return false;
    }

    template<typename T> static T clamp(T min, T max, T val) {
        if(val < min)
            return min;
        if(val > max)
            return max;
        return val;
    }

    template<typename T> static bool isPOT(T n) {
        return n && !(n & (n - 1));
    }

    template<class T> static typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
        almostEqual(T x, T y, int32_t ulp = 4) {
        return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
               || std::abs(x - y) < std::numeric_limits<T>::min();
    }

    template<typename T>
    typename T::size_type levensteinDistance(const T& source, const T& target) {
        if (source.size() > target.size()) {
            return levensteinDistance(target, source);
        }

        using TSizeType = typename T::size_type;
        const TSizeType min_size = source.size(), max_size = target.size();
        std::vector<TSizeType> lev_dist(min_size + 1);

        for (TSizeType i = 0; i <= min_size; ++i) {
            lev_dist[i] = i;
        }

        for (TSizeType j = 1; j <= max_size; ++j) {
            TSizeType previous_diagonal = lev_dist[0], previous_diagonal_save;
            ++lev_dist[0];

            for (TSizeType i = 1; i <= min_size; ++i) {
                previous_diagonal_save = lev_dist[i];
                if (source[i - 1] == target[j - 1]) {
                    lev_dist[i] = previous_diagonal;
                } else {
                    lev_dist[i] = std::min(std::min(lev_dist[i - 1], lev_dist[i]), previous_diagonal) + 1;
                }
                previous_diagonal = previous_diagonal_save;
            }
        }

        return lev_dist[min_size];
    }

    static ARK_API uint32_t log2(uint32_t x);

//  [[script::bindings::auto]]
    static ARK_API float sin(float x);
//  [[plugin::function("sin")]]
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> sin(const sp<Numeric>& x);
//  [[script::bindings::auto]]
    static ARK_API float cos(float x);
//  [[plugin::function("cos")]]
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> cos(const sp<Numeric>& x);

//  [[plugin::function("min")]]
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> min(const sp<Numeric>& a1, const sp<Numeric>& a2);
//  [[plugin::function("max")]]
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> max(const sp<Numeric>& a1, const sp<Numeric>& a2);

//  [[script::bindings::auto]]
    static ARK_API float acos(float x);
//  [[plugin::function("acos")]]
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> acos(const sp<Numeric>& x);
//  [[script::bindings::auto]]
    static ARK_API float atan2(float y, float x);
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> atan2(const sp<Numeric>& dy, const sp<Numeric>& dx);
//  [[script::bindings::auto]]
    static ARK_API float radians(float degree);

//  [[script::bindings::auto]]
    static ARK_API float tanh(float x);

    static ARK_API int32_t floor(float x);
    static ARK_API int32_t round(float x);

//  [[script::bindings::auto]]
    static ARK_API float randf();
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> randv();

    static ARK_API uint32_t hypot(int32_t dx, int32_t dy);
//  [[script::bindings::auto]]
    static ARK_API float hypot(float dx, float dy);
//  [[script::bindings::auto]]
    static ARK_API float sqrt(float number);
//  [[plugin::function("sqrt")]]
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> sqrt(const sp<Numeric>& number);

//  [[script::bindings::auto]]
    static ARK_API float distance(const V2& lvalue, const V2& rvalue);
//  [[script::bindings::auto]]
    static ARK_API float distance(const V3& lvalue, const V3& rvalue);
//  [[script::bindings::auto]]
    static ARK_API float distance(const V4& lvalue, const V4& rvalue);

//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> distance(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue);
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> distance(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> distance(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue);

//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> dot(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue);
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> dot(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> dot(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue);

//  [[script::bindings::auto]]
    static ARK_API sp<Numeric> lerp(const sp<Numeric>& a, const sp<Numeric>& b, const sp<Numeric>& t);
//  [[script::bindings::auto]]
    static ARK_API sp<Vec2> lerp(const sp<Vec2>& a, const sp<Vec2>& b, const sp<Numeric>& t);
//  [[script::bindings::auto]]
    static ARK_API sp<Vec3> lerp(const sp<Vec3>& a, const sp<Vec3>& b, const sp<Numeric>& t);
//  [[script::bindings::auto]]
    static ARK_API sp<Vec4> lerp(const sp<Vec4>& a, const sp<Vec4>& b, const sp<Numeric>& t);

//  [[script::bindings::auto]]
    static ARK_API V3 quadratic(float a, float b, float c);
//  [[script::bindings::auto]]
    static ARK_API V2 projectile(float dx, float dy, float v, float g, uint32_t sid);

    static void vibrate(float s0, float v0, float s1, float v1, float& o, float& a, float& t0, float& t1);

    static ARK_API const float PI;
    static ARK_API const float PIx2;
    static ARK_API const float PI_2;
    static ARK_API const float PI_4;
};

}

#endif
