#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API Math {
public:
    template<typename T> static bool between(T a1, T a2, T val) {
        return a1 <= a2 ? (a1 <= val && val <= a2) : (a2 <= val && val <= a1);
    }

    template<typename T, typename U> static std::remove_reference_t<T> floorDiv(T value, U mod) {
        return floorDiv_sfinae(value, mod, nullptr);
    }

    template<typename T, typename U> static std::remove_reference_t<T> floorDiv_sfinae(T value, U mod, decltype(value.floorDiv(mod))*) {
        return value.floorDiv(mod);
    }

    template<typename T, typename U> static std::remove_reference_t<T> floorDiv_sfinae(T value, U mod, std::enable_if_t<std::is_integral_v<T> && std::is_integral_v<U>>*) {
        return static_cast<std::remove_reference_t<T>>(std::floor(value /  static_cast<float>(mod)));
    }

    template<typename T, typename U> static std::remove_reference_t<T> floorDiv_sfinae(T value, U mod, ...) {
        return std::floor(value /  mod);
    }

    template<typename T, typename U> static std::remove_reference_t<T> floorMod(T value, U mod) {
        return floorMod_sfinae(value, mod, nullptr);
    }

    template<typename T, typename U> static std::remove_reference_t<T> floorMod_sfinae(T value, U mod, decltype(value.floorMod(mod))*) {
        return value.floorMod(mod);
    }

    template<typename T, typename U> static std::remove_reference_t<T> floorMod_sfinae(T value, U mod, std::enable_if_t<std::is_integral_v<T> && std::is_integral_v<U>>*) {
        return (value % static_cast<T>(mod) + static_cast<T>(mod)) % static_cast<T>(mod);
    }

    template<typename T, typename U> static std::remove_reference_t<T> floorMod_sfinae(T value, U mod, ...) {
        return value - std::floor(value / mod);
    }

    template<typename T> static std::remove_reference_t<T> divmod(T value, T mod, T& remainder) {
        const auto quot = floorDiv(value,  mod);
        remainder = value - quot * mod;
        return quot;
    }

    template<typename T> static std::remove_reference_t<T> mod(T value, T mod) {
        auto v = value - floorDiv(value,  mod) * mod;
        return signEquals(v, mod) ? v : v + mod;
    }

    template<typename T, typename U> static std::remove_reference_t<T> modFloor(T value, U mod) {
        return floorDiv(value, mod) * mod;
    }

    template<typename T, typename U> static void modBetween(T start, T end, U mod, T& modFloor, T& modCeiling) {
        modFloor = Math::modFloor<T>(start, mod);
        modCeiling = Math::modFloor<T>(end, mod) + mod;
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

    template<typename T> static typename T::size_type levensteinDistance(const T& source, const T& target) {
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

    template<typename T, typename U> static std::pair<T, typename T::size_type> levensteinNearest(const T& source, const U& candidates) {
        T nearest;
        typename T::size_type ldmin = std::numeric_limits<typename T::size_type>::max();
        for(const auto& j : candidates) {
            typename T::size_type ld = levensteinDistance(source, j);
            if(ld < ldmin) {
                ldmin = ld;
                nearest = j;
            }
        }
        return std::make_pair(nearest, ldmin);
    }

    template<typename T, typename U> static void hashCombine(T& seed, const U& v) {
        std::hash<U> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    static uint32_t log2(uint32_t x);

    static float abs(float x);
    static V2 abs(const V2& x);
    static V3 abs(const V3& x);
    static V4 abs(const V4& x);

//  [[script::bindings::auto]]
    static float sin(float x);
//  [[plugin::function("sin")]]
//  [[script::bindings::auto]]
    static sp<Numeric> sin(const sp<Numeric>& x);
//  [[script::bindings::auto]]
    static float cos(float x);
//  [[plugin::function("cos")]]
//  [[script::bindings::auto]]
    static sp<Numeric> cos(const sp<Numeric>& x);

//  [[plugin::function("min")]]
//  [[script::bindings::auto]]
    static sp<Numeric> min(sp<Numeric> a1, sp<Numeric> a2);
//  [[plugin::function("max")]]
//  [[script::bindings::auto]]
    static sp<Numeric> max(sp<Numeric> a1, sp<Numeric> a2);

//  [[script::bindings::auto]]
    static float acos(float x);
//  [[plugin::function("acos")]]
//  [[script::bindings::auto]]
    static sp<Numeric> acos(const sp<Numeric>& x);
//  [[script::bindings::auto]]
    static float atan2(float y, float x);
//  [[script::bindings::auto]]
    static sp<Numeric> atan2(sp<Numeric> y, sp<Numeric> x);
//  [[script::bindings::auto]]
    static float radians(float degree);

//  [[script::bindings::auto]]
    static float tanh(float x);

    static int32_t floor(float x);
    static float round(float x);
    static V2 round(const V2& v);
    static V3 round(const V3& v);
    static V4 round(const V4& v);

//  [[script::bindings::auto]]
    static float randf();
//  [[script::bindings::auto]]
    static sp<Numeric> randv();

//  [[script::bindings::auto]]
    static uint32_t hypot(int32_t dx, int32_t dy);
//  [[script::bindings::auto]]
    static float hypot(float dx, float dy);
//  [[script::bindings::auto]]
    static float sqrt(float number);
//  [[plugin::function("sqrt")]]
//  [[script::bindings::auto]]
    static sp<Numeric> sqrt(sp<Numeric> number);

//  [[script::bindings::auto]]
    static float distance(const V2& lvalue, const V2& rvalue);
//  [[script::bindings::auto]]
    static float distance(const V3& lvalue, const V3& rvalue);
//  [[script::bindings::auto]]
    static float distance(const V4& lvalue, const V4& rvalue);

//  [[script::bindings::auto]]
    static sp<Numeric> distance(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue);
//  [[script::bindings::auto]]
    static sp<Numeric> distance(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//  [[script::bindings::auto]]
    static sp<Numeric> distance(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue);

//  [[script::bindings::auto]]
    static sp<Numeric> dot(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue);
//  [[script::bindings::auto]]
    static sp<Numeric> dot(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//  [[script::bindings::auto]]
    static sp<Numeric> dot(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue);

//  [[script::bindings::auto]]
    static float lerp(float a, float b, float t);
    static float lerp(float a, float b, float t0, float t1, float t);
//  [[script::bindings::auto]]
    static sp<Numeric> lerp(sp<Numeric> a, sp<Numeric> b, sp<Numeric> t);
//  [[script::bindings::auto]]
    static sp<Vec2> lerp(sp<Vec2> a, sp<Vec2> b, sp<Numeric> t);
//  [[script::bindings::auto]]
    static sp<Vec3> lerp(sp<Vec3> a, sp<Vec3> b, sp<Numeric> t);
//  [[script::bindings::auto]]
    static sp<Vec4> lerp(sp<Vec4> a, sp<Vec4> b, sp<Numeric> t);

    [[deprecated]]
//  [[script::bindings::auto]]
    static V3 quadratic(float a, float b, float c);
    [[deprecated]]
//  [[script::bindings::auto]]
    static V2 projectile(float dx, float dy, float v, float g, uint32_t sid);

    static uint32_t hash32(uint32_t x);
    static uint32_t hash64(uint64_t key);

    static V2 normalize(const V2& v2);
    static V3 normalize(const V3& v2);
    static V4 normalize(const V4& v2);

    static V4 slerp(const V4& x, const V4& y, float t);

    static void vibrate(float s0, float v0, float s1, float v1, float& o, float& a, float& t0, float& t1);

    static const float PI;
    static const float PIx2;
    static const float PI_2;
    static const float PI_4;
};

}
