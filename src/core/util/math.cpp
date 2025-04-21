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
#include "core/types/safe_var.h"
#include "core/util/operators.h"
#include "core/util/log.h"

#include "graphics/base/v4.h"

namespace ark {

namespace {

template<typename T> class Randomizer {
public:
    Randomizer()
        : _generator(_random_device()), _distribution(std::numeric_limits<T>::min(), std::numeric_limits<T>::max()) {
    }

    T rand()
    {
        return _distribution(_generator);
    }

private:
    std::random_device _random_device;
    std::mt19937 _generator;
    std::uniform_int_distribution<> _distribution;
};

class VolatileRandfv final : public Numeric {
public:
    VolatileRandfv(sp<Numeric> a, sp<Numeric> b)
        : _a(std::move(a)), _b(std::move(b), 1.0f)
    {
    }

    float val() override
    {
        const float a = _a.val();
        const float b = _b.val();
        return Math::randf() * (b - a) + a;
    }

    bool update(const uint64_t timestamp) override
    {
        return true;
    }

private:
    SafeVar<Numeric> _a;
    SafeVar<Numeric> _b;
};

class Randfv final : public Numeric {
public:
    Randfv(sp<Numeric> a, sp<Numeric> b)
        : _a(std::move(a)), _b(std::move(b), 1.0f), _value(doGenerate())
    {
    }

    float val() override
    {
        const float a = _a.val();
        const float b = _b.val();
        return Math::randf() * (b - a) + a;
    }

    bool update(const uint64_t timestamp) override
    {
        const bool dirty = _timestamp.update(timestamp);
        if(dirty)
            _value = doGenerate();
        return dirty;
    }

private:
    float doGenerate() const
    {
        const float a = _a.val();
        const float b = _b.val();
        return Math::randf() * (b - a) + a;
    }

private:
    SafeVar<Numeric> _a;
    SafeVar<Numeric> _b;

    Timestamp _timestamp;
    float _value;
};

}

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

V2 Math::round(const V2& x)
{
    return {std::round(x.x()), std::round(x.y())};
}

V3 Math::round(const V3& x)
{
    return {std::round(x.x()), std::round(x.y()), std::round(x.z())};
}

V4 Math::round(const V4& x)
{
    return {std::round(x.x()), std::round(x.y()), std::round(x.z()), std::round(x.w())};
}

int32_t Math::rand()
{
    static Randomizer<int32_t> randomizer;
    return randomizer.rand();
}

float Math::randf()
{
    return static_cast<float>(static_cast<int64_t>(rand()) - std::numeric_limits<int32_t>::min()) / static_cast<float>(std::numeric_limits<uint32_t>::max());
}

sp<Numeric> Math::randfv(sp<Numeric> a, sp<Numeric> b, const bool isVolatile)
{
    if(isVolatile)
        return sp<Numeric>::make<VolatileRandfv>(std::move(a), std::move(b));
    return sp<Numeric>::make<Randfv>(std::move(a), std::move(b));
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

float Math::distance(const V2& lvalue, const V2& rvalue)
{
    return (lvalue - rvalue).hypot();
}

float Math::distance(const V3& lvalue, const V3& rvalue)
{
    return (lvalue - rvalue).hypot();
}

float Math::distance(const V4& lvalue, const V4& rvalue)
{
    return (lvalue - rvalue).hypot();
}

sp<Numeric> Math::distance(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Distance<V2>>>(lvalue, rvalue);
}

sp<Numeric> Math::distance(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Distance<V3>>>(lvalue, rvalue);
}

sp<Numeric> Math::distance(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::Distance<V4>>>(lvalue, rvalue);
}

sp<Numeric> Math::dot(sp<Vec2> lvalue, sp<Vec2> rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Dot<V2>>>(std::move(lvalue), std::move(rvalue));
}

sp<Numeric> Math::dot(sp<Vec3> lvalue, sp<Vec3> rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Dot<V3>>>(std::move(lvalue), std::move(rvalue));
}

sp<Numeric> Math::dot(sp<Vec4> lvalue, sp<Vec4> rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::Dot<V4>>>(std::move(lvalue), std::move(rvalue));
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

V2 Math::normalize(const V2& v2)
{
    const glm::vec2 n = glm::normalize(glm::vec2(v2.x(), v2.y()));
    return {n.x, n.y};
}

V3 Math::normalize(const V3& v3)
{
    const glm::vec3 n = glm::normalize(glm::vec3(v3.x(), v3.y(), v3.z()));
    return {n.x, n.y, n.z};
}

V4 Math::normalize(const V4& v4)
{
    const glm::vec4 n = glm::normalize(glm::vec4(v4.x(), v4.y(), v4.z(), v4.w()));
    return {n.x, n.y, n.z, n.w};
}

V4 Math::slerp(const V4& x, const V4& y, const float t)
{
    const glm::quat n = glm::slerp(glm::quat(x.w(), x.x(), x.y(), x.z()), glm::quat(y.w(), y.x(), y.y(), y.z()), t);
    return {n.x, n.y, n.z, n.w};
}

}
