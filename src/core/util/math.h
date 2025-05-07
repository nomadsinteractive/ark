#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

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

    template<class T> static std::enable_if_t<!std::numeric_limits<T>::is_integer, bool> almostEqual(T x, T y, int32_t ulp = 4) {
        return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
               || std::abs(x - y) < std::numeric_limits<T>::min();
    }

    template<typename T> static typename T::size_type levensteinDistance(const T& source, const T& target) {
        if (source.size() > target.size()) {
            return levensteinDistance(target, source);
        }

        using TSizeType = typename T::size_type;
        const TSizeType min_size = source.size(), max_size = target.size();
        Vector<TSizeType> lev_dist(min_size + 1);

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

    static int32_t abs(int32_t x);
    static float abs(float x);
    static V2 abs(const V2& x);
    static V3 abs(const V3& x);
    static V4 abs(const V4& x);

//  [[script::bindings::auto]]
    static float sin(float x);
//  [[script::bindings::auto]]
    static sp<Numeric> sin(const sp<Numeric>& x);
//  [[script::bindings::auto]]
    static float cos(float x);
//  [[script::bindings::auto]]
    static sp<Numeric> cos(const sp<Numeric>& x);

//  [[script::bindings::auto]]
    static sp<Numeric> min(sp<Numeric> a1, sp<Numeric> a2);
//  [[script::bindings::auto]]
    static sp<Numeric> max(sp<Numeric> a1, sp<Numeric> a2);

//  [[script::bindings::auto]]
    static float acos(float x);
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

//  [[script::bindings::auto]]
    static int32_t rand();
//  [[script::bindings::auto]]
    static float randf();
//  [[script::bindings::auto]]
    static sp<Numeric> randfv(sp<Numeric> a = nullptr, sp<Numeric> b = nullptr, bool isVolatile = true);

//  [[script::bindings::auto]]
    static float hypot(float dx, float dy);
//  [[script::bindings::auto]]
    static float hypot2(float dx, float dy);
//  [[script::bindings::auto]]
    static float sqrt(float x);
//  [[script::bindings::auto]]
    static sp<Numeric> sqrt(sp<Numeric> x);

//  [[script::bindings::auto]]
    static float distance(V2 a, V2 b);
//  [[script::bindings::auto]]
    static float distance(V3 a, V3 b);
//  [[script::bindings::auto]]
    static float distance(V4 a, V4 b);

//  [[script::bindings::auto]]
    static sp<Numeric> distance(sp<Vec2> a, sp<Vec2> b);
//  [[script::bindings::auto]]
    static sp<Numeric> distance(sp<Vec3> a, sp<Vec3> b);
//  [[script::bindings::auto]]
    static sp<Numeric> distance(sp<Vec4> a, sp<Vec4> b);

//  [[script::bindings::auto]]
    static sp<Numeric> dot(sp<Vec2> a, sp<Vec2> b);
//  [[script::bindings::auto]]
    static sp<Numeric> dot(sp<Vec3> a, sp<Vec3> b);
//  [[script::bindings::auto]]
    static sp<Numeric> dot(sp<Vec4> a, sp<Vec4> b);

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

    static uint32_t hash32(uint32_t x);
    static uint32_t hash64(uint64_t x);

    static float round(float x);
    static V2 round(V2 x);
    static V3 round(V3 x);
    static V4 round(V4 x);

    static float normalize(float v1);
    static V2 normalize(V2 v2);
    static V3 normalize(V3 v3);
    static V4 normalize(V4 v4);

    static float dot(float a, float b);
    static float dot(V2 a, V2 b);
    static float dot(V3 a, V3 b);
    static float dot(V4 a, V4 b);

    static float hypot(float v1);
    static float hypot(V2 v2);
    static float hypot(V3 v3);
    static float hypot(V4 v4);

    static float hypot2(float v1);
    static float hypot2(V2 v2);
    static float hypot2(V3 v3);
    static float hypot2(V4 v4);

    static V4 slerp(V4 x, V4 y, float t);

    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float PI_HALF = PI / 2.0f;
    static constexpr float PI_QUARTER = PI / 4.0f;
    static constexpr float TAU = PI * 2.0f;
};

}
