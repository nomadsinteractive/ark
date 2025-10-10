#include "core/util/math.h"

#include <algorithm>
#include <math.h>
#include <random>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_common.hpp>

#include "core/inf/variable.h"
#include "core/impl/variable/lerp.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/types/optional_var.h"
#include "core/util/operators.h"
#include "core/util/log.h"

#include "graphics/base/v4.h"

namespace ark {

uint32_t Math::log2(uint32_t x)
{
    constexpr uint32_t tab32[32] = {
        0,  9,  1, 10, 13, 21,  2, 29,
       11, 14, 16, 18, 22, 25,  3, 30,
        8, 12, 20, 28, 15, 17, 24,  7,
       19, 27, 23,  6, 26,  5,  4, 31};
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return tab32[static_cast<uint32_t>((x * 0x07C4ACDD) >> 27)];
}

int32_t Math::abs(const int32_t x)
{
    return std::abs(x);
}

float Math::abs(const float x)
{
    return std::abs(x);
}

V2 Math::abs(const V2& x)
{
    return {std::abs(x.x()), std::abs(x.y())};
}

V3 Math::abs(const V3& x)
{
    return {std::abs(x.x()), std::abs(x.y()), std::abs(x.z())};
}

V4 Math::abs(const V4& x)
{
    return {std::abs(x.x()), std::abs(x.y()), std::abs(x.z()), std::abs(x.w())};
}

float Math::sin(const float x)
{
    return std::sin(x);
}

sp<Numeric> Math::sin(const sp<Numeric>& x)
{
    return sp<Numeric>::make<VariableOP1<float>>(static_cast<float(*)(float)>(Math::sin), x);
}

float Math::cos(const float x)
{
    return std::cos(x);
}

sp<Numeric> Math::cos(const sp<Numeric>& x)
{
    return sp<Numeric>::make<VariableOP1<float>>(static_cast<float(*)(float)>(Math::cos), x);
}

sp<Numeric> Math::min(sp<Numeric> a1, sp<Numeric> a2)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Min<float>>>(std::move(a1), std::move(a2));
}

sp<Numeric> Math::max(sp<Numeric> a1, sp<Numeric> a2)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Max<float>>>(std::move(a1), std::move(a2));
}

float Math::acos(const float x)
{
    return std::acos(x);
}

sp<Numeric> Math::acos(const sp<Numeric>& x)
{
    return sp<Numeric>::make<VariableOP1<float>>(static_cast<float(*)(float)>(Math::acos), x);
}

float Math::atan2(const float y, const float x)
{
    return std::atan2(y, x);
}

sp<Numeric> Math::atan2(sp<Numeric> y, sp<Numeric> x)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Atan2>>(std::move(y), std::move(x));
}

float Math::radians(const float degree)
{
    return degree / 180.0f * PI;
}

float Math::tanh(const float x)
{
    return std::tanh(x);
}

int32_t Math::floor(const float x)
{
    return static_cast<int32_t>(x);
}

float Math::round(const float x)
{
    return std::round(x);
}

V2 Math::round(const V2 x)
{
    return {std::round(x.x()), std::round(x.y())};
}

V3 Math::round(const V3 x)
{
    return {std::round(x.x()), std::round(x.y()), std::round(x.z())};
}

V4 Math::round(const V4 x)
{
    return {std::round(x.x()), std::round(x.y()), std::round(x.z()), std::round(x.w())};
}

float Math::hypot(const float dx, const float dy)
{
    return sqrt(dx * dx + dy * dy);
}

float Math::hypot2(const float dx, const float dy)
{
    return dx * dx + dy * dy;
}

float Math::sqrt(const float x)
{
    CHECK(x >= 0, "Illegal argument, negative value(%.2f)", x);
    return std::sqrt(x);
}

sp<Numeric> Math::sqrt(sp<Numeric> x)
{
    return sp<Numeric>::make<VariableOP1<float>>(static_cast<float(*)(float)>(Math::sqrt), std::move(x));
}

float Math::distance(const V2 a, const V2 b)
{
    return hypot(a - b);
}

float Math::distance(const V3 a, const V3 b)
{
    return hypot(a - b);
}

float Math::distance(const V4 a, const V4 b)
{
    return hypot(a - b);
}

sp<Numeric> Math::distance(sp<Vec2> a, sp<Vec2> b)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Distance<V2>>>(std::move(a), std::move(b));
}

sp<Numeric> Math::distance(sp<Vec3> a, sp<Vec3> b)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Distance<V3>>>(std::move(a), std::move(b));
}

sp<Numeric> Math::distance(sp<Vec4> a, sp<Vec4> b)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::Distance<V4>>>(std::move(a), std::move(b));
}

sp<Numeric> Math::dot(sp<Vec2> a, sp<Vec2> b)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Dot<V2>>>(std::move(a), std::move(b));
}

sp<Numeric> Math::dot(sp<Vec3> a, sp<Vec3> b)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Dot<V3>>>(std::move(a), std::move(b));
}

sp<Numeric> Math::dot(sp<Vec4> a, sp<Vec4> b)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::Dot<V4>>>(std::move(a), std::move(b));
}

float Math::lerp(const float a, const float b, const float t)
{
    return a + (b - a) * t;
}

float Math::lerp(const float a, const float b, const float t0, const float t1, const float t)
{
    const float nt = t0 == t1 ? 0 : (t - t0) / (t1 - t0);
    return lerp(a, b, nt);
}

sp<Numeric> Math::lerp(sp<Numeric> a, sp<Numeric> b, sp<Numeric> t)
{
    return sp<Lerp<float, float>>::make(std::move(a), std::move(b), std::move(t));
}

sp<Vec2> Math::lerp(sp<Vec2> a, sp<Vec2> b, sp<Numeric> t)
{
    return sp<Lerp<V2, float>>::make(std::move(a), std::move(b), std::move(t));
}

sp<Vec3> Math::lerp(sp<Vec3> a, sp<Vec3> b, sp<Numeric> t)
{
    return sp<Lerp<V3, float>>::make(std::move(a), std::move(b), std::move(t));
}

sp<Vec4> Math::lerp(sp<Vec4> a, sp<Vec4> b, sp<Numeric> t)
{
    return sp<Lerp<V4, float>>::make(std::move(a), std::move(b), std::move(t));
}

uint32_t Math::hash32(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

uint32_t Math::hash64(uint64_t x)
{
    x = (~x) + (x << 18);
    x = x ^ (x >> 31);
    x = x * 21;
    x = x ^ (x >> 11);
    x = x + (x << 6);
    x = x ^ (x >> 22);
    return x;
}

uint32_t Math::hashf(const float x)
{
    return hash32(*reinterpret_cast<const uint32_t *>(&x));
}

uint32_t Math::hash(const uint32_t x)
{
    return hash32(x);
}

uint32_t Math::hash(const uint64_t x)
{
    return hash64(x);
}

uint32_t Math::hash(const float x)
{
    return hashf(x);
}

uint32_t Math::hash(const V2i x)
{
    uint32_t val = hash32(x[0]);
    hashCombine(val, hash32(x[1]));
    return val;
}

uint32_t Math::hash(const V3i x)
{
    uint32_t val = hash32(x[0]);
    hashCombine(val, hash32(x[1]));
    hashCombine(val, hash32(x[2]));
    return val;
}

uint32_t Math::hash(const V4i x)
{
    uint32_t val = hash32(x[0]);
    hashCombine(val, hash32(x[1]));
    hashCombine(val, hash32(x[2]));
    hashCombine(val, hash32(x[3]));
    return val;
}

uint32_t Math::hash(const V2 x)
{
    uint32_t val = hashf(x.x());
    hashCombine(val, hashf(x.y()));
    return val;
}

uint32_t Math::hash(const V3 x)
{
    uint32_t val = hashf(x.x());
    hashCombine(val, hashf(x.y()));
    hashCombine(val, hashf(x.z()));
    return val;
}

uint32_t Math::hash(const V4 x)
{
    uint32_t val = hashf(x.x());
    hashCombine(val, hashf(x.y()));
    hashCombine(val, hashf(x.z()));
    hashCombine(val, hashf(x.w()));
    return val;
}

float Math::normalize(const float v1)
{
    return v1 >= 0.0f ? 1.0f : -1.0f;
}

V2 Math::normalize(const V2 v2)
{
    const glm::vec2 n = glm::normalize(glm::vec2(v2.x(), v2.y()));
    return {n.x, n.y};
}

V3 Math::normalize(const V3 v3)
{
    const glm::vec3 n = glm::normalize(glm::vec3(v3.x(), v3.y(), v3.z()));
    return {n.x, n.y, n.z};
}

V4 Math::normalize(const V4 v4)
{
    const glm::vec4 n = glm::normalize(glm::vec4(v4.x(), v4.y(), v4.z(), v4.w()));
    return {n.x, n.y, n.z, n.w};
}

float Math::dot(const float a, const float b)
{
    return a * b;
}

float Math::dot(const V2 a, const V2 b)
{
    return a.x() * b.x() + a.y() * b.y();
}

float Math::dot(const V3 a, const V3 b)
{
    return a.x() * b.x() + a.y() * b.y() + a.z() * b.z();
}

float Math::dot(const V4 a, const V4 b)
{
    return a.x() * b.x() + a.y() * b.y() + a.z() * b.z() + a.w() * b.w();
}

float Math::hypot(const float v1)
{
    return v1;
}

float Math::hypot(const V2 v2)
{
    return sqrt(hypot2(v2));
}

float Math::hypot(const V3 v3)
{
    return sqrt(hypot2(v3));
}

float Math::hypot(const V4 v4)
{
    return sqrt(hypot2(v4));
}

float Math::hypot2(const float v1)
{
    return v1 * v1;
}

float Math::hypot2(const V2 v2)
{
    return v2.x() * v2.x() + v2.y() * v2.y();
}

float Math::hypot2(const V3 v3)
{
    return v3.x() * v3.x() + v3.y() * v3.y() + v3.z() * v3.z();
}

float Math::hypot2(const V4 v4)
{
    return v4.x() * v4.x() + v4.y() * v4.y() + v4.z() * v4.z() + v4.w() * v4.w();
}

V4 Math::slerp(const V4 x, const V4 y, const float t)
{
    const glm::quat n = glm::slerp(glm::quat(x.w(), x.x(), x.y(), x.z()), glm::quat(y.w(), y.x(), y.y(), y.z()), t);
    return {n.x, n.y, n.z, n.w};
}

}
