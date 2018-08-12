#ifndef ARK_CORE_UTIL_MATH_H_
#define ARK_CORE_UTIL_MATH_H_

#include <cstdint>
#include <cmath>
#include <limits>
#include <type_traits>

#include "core/base/api.h"
#include "graphics/base/v3.h"

namespace ark {

class Math {
public:

    template<typename T> static T abs(T x) {
        return x > 0 ? x : -x;
    }

    template<typename T> static bool between(T a1, T a2, T val) {
        return a1 <= a2 ? (a1 <= val && val <= a2) : (a2 <= val && val <= a1);
    }

    template<typename T> static T divmod(T value, T mod, T& remainder) {
        T quot = static_cast<int32_t>(value /  mod);
        remainder = value - quot * mod;
        return quot;
    }

    template<typename T> static T mod(T value, T mod) {
        DCHECK(mod >= 1, "Argument mod's value must be greater than one");
        T v = value - static_cast<int32_t>(value /  mod) * T(mod);
        return v >= 0 ? v : v + mod;
    }

    template<typename T> static T modFloor(T value, T mod) {
        return value - Math::mod<T>(value, mod);
    }

    template<typename T> static void modBetween(T start, T end, T mod, T& modFloor, T& modCelling) {
        T m = Math::mod<T>(start, mod);
        modFloor = start - m;
        modCelling = Math::modFloor<T>(end, mod) + (m == 0 ? 0 : mod);
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

    template<class T> static typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
        almostEqual(T x, T y, int32_t ulp = 4) {
        return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
               || std::abs(x - y) < std::numeric_limits<T>::min();
    }

    static ARK_API uint32_t log2(uint32_t x);

//  [[script::bindings::auto]]
    static ARK_API float sin(float x);
//  [[script::bindings::auto]]
    static ARK_API float cos(float x);

//  [[script::bindings::auto]]
    static ARK_API float acos(float x);
//  [[script::bindings::auto]]
    static ARK_API float atan2(float y, float x);
//  [[script::bindings::auto]]
    static ARK_API float radians(float degree);

//  [[script::bindings::auto]]
    static ARK_API float tanh(float x);

    static ARK_API int32_t floor(float x);
    static ARK_API int32_t round(float x);

//  [[script::bindings::auto]]
    static ARK_API float randf();

    static ARK_API uint32_t hypot(int32_t dx, int32_t dy);
//  [[script::bindings::auto]]
    static ARK_API float hypot(float dx, float dy);
//  [[script::bindings::auto]]
    static ARK_API float sqrt(float number);

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
