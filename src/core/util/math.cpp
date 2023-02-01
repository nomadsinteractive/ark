#include "core/util/math.h"

#include <algorithm>
#include <math.h>
#include <stdlib.h>

#include <glm/glm.hpp>

#include "core/inf/variable.h"
#include "core/impl/variable/interpolate.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"
#include "core/util/log.h"

#include "graphics/base/v4.h"
#include "graphics/impl/vec/vec2_impl.h"
#include "graphics/impl/vec/vec3_impl.h"
#include "graphics/impl/vec/vec4_impl.h"


namespace ark {

namespace {

class RandNumeric : public Numeric {
public:
    virtual float val() override {
        return Math::randf();
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return true;
    }
};

}

const float Math::PI = 3.14159265358979323846f;
const float Math::PIx2 = Math::PI * 2.0f;
const float Math::PI_2 = Math::PI / 2.0f;
const float Math::PI_4 = Math::PI / 4.0f;

uint32_t Math::log2(uint32_t value)
{
    static const uint32_t tab32[32] = {
        0,  9,  1, 10, 13, 21,  2, 29,
       11, 14, 16, 18, 22, 25,  3, 30,
        8, 12, 20, 28, 15, 17, 24,  7,
       19, 27, 23,  6, 26,  5,  4, 31};
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return tab32[static_cast<uint32_t>((value * 0x07C4ACDD) >> 27)];
}

float Math::abs(float x)
{
    return std::abs(x);
}

V2 Math::abs(const V2& x)
{
    return V2(std::abs(x.x()), std::abs(x.y()));
}

V3 Math::abs(const V3& x)
{
    return V3(std::abs(x.x()), std::abs(x.y()), std::abs(x.z()));
}

V4 Math::abs(const V4& x)
{
    return V4(std::abs(x.x()), std::abs(x.y()), std::abs(x.z()), std::abs(x.w()));
}

float Math::sin(float x)
{
    return std::sin(x);
}

sp<Numeric> Math::sin(const sp<Numeric>& x)
{
    return sp<VariableOP1<float>>::make(static_cast<float(*)(float)>(Math::sin), x);
}

float Math::cos(float x)
{
    return std::cos(x);
}

sp<Numeric> Math::cos(const sp<Numeric>& x)
{
    return sp<VariableOP1<float>>::make(static_cast<float(*)(float)>(Math::cos), x);
}

sp<Numeric> Math::min(const sp<Numeric>& a1, const sp<Numeric>& a2)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Min<float>>>::make(a1, a2);
}

sp<Numeric> Math::max(const sp<Numeric>& a1, const sp<Numeric>& a2)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Max<float>>>::make(a1, a2);
}

float Math::acos(float x)
{
    return std::acos(x);
}

sp<Numeric> Math::acos(const sp<Numeric>& x)
{
    return sp<VariableOP1<float>>::make(static_cast<float(*)(float)>(Math::acos), x);
}

float Math::atan2(float y, float x)
{
    return std::atan2(y, x);
}

sp<Numeric> Math::atan2(const sp<Numeric>& dy, const sp<Numeric>& dx)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Atan2>>::make(dy, dx);
}

float Math::radians(float degree)
{
    return degree / 180.0f * PI;
}

float Math::tanh(float x)
{
    return std::tanh(x);
}

int32_t Math::floor(float x)
{
    return static_cast<int32_t>(x);
}

float Math::round(float x)
{
    return std::round(x);
}

V2 Math::round(const V2& v)
{
    return V2(std::round(v.x()), std::round(v.y()));
}

V3 Math::round(const V3& v)
{
    return V3(std::round(v.x()), std::round(v.y()), std::round(v.z()));
}

V4 Math::round(const V4& v)
{
    return V4(std::round(v.x()), std::round(v.y()), std::round(v.z()), std::round(v.w()));
}

float Math::randf()
{
    return rand() / static_cast<float>(RAND_MAX);
}

sp<Numeric> Math::randv()
{
    return sp<RandNumeric>::make();
}

uint32_t Math::hypot(int32_t dx, int32_t dy)
{
    int32_t min, max, approx;

    if(dx < 0) dx = -dx;
    if(dy < 0) dy = -dy;

    if(dx < dy)
    {
        min = dx;
        max = dy;
    }
    else
    {
        min = dy;
        max = dx;
    }

    approx = (max * 1007) + (min * 441);
    if(max < (min << 4))
        approx -= (max * 40);

    return ((approx + 512) >> 10);
}

float Math::hypot(float dx, float dy)
{
    return sqrt(dx * dx + dy * dy);
}

float Math::sqrt(float x)
{
    DCHECK(x >= 0, "Illegal argument, negative value(%.2f)", x);
    return std::sqrt(x);
}

sp<Numeric> Math::sqrt(const sp<Numeric>& x)
{
    return sp<VariableOP1<float>>::make(static_cast<float(*)(float)>(Math::sqrt), x);
}

float Math::distance(const V2& lvalue, const V2& rvalue)
{
    return (lvalue - rvalue).length();
}

float Math::distance(const V3& lvalue, const V3& rvalue)
{
    return (lvalue - rvalue).length();
}

float Math::distance(const V4& lvalue, const V4& rvalue)
{
    return (lvalue - rvalue).length();
}

sp<Numeric> Math::distance(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue)
{
    return sp<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Distance<V2>>>::make(lvalue, rvalue);
}

sp<Numeric> Math::distance(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Distance<V3>>>::make(lvalue, rvalue);
}

sp<Numeric> Math::distance(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::Distance<V4>>>::make(lvalue, rvalue);
}

sp<Numeric> Math::dot(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue)
{
    return sp<VariableOP2<sp<Vec2>, sp<Vec2>, Operators::Dot<V2>>>::make(lvalue, rvalue);
}

sp<Numeric> Math::dot(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<VariableOP2<sp<Vec3>, sp<Vec3>, Operators::Dot<V3>>>::make(lvalue, rvalue);
}

sp<Numeric> Math::dot(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue)
{
    return sp<VariableOP2<sp<Vec4>, sp<Vec4>, Operators::Dot<V4>>>::make(lvalue, rvalue);
}

float Math::lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float Math::lerp(float a, float b, float t0, float t1, float t)
{
    float nt = t0 == t1 ? 0 : (t - t0) / (t1 - t0);
    return lerp(a, b, nt);
}

sp<Numeric> Math::lerp(sp<Numeric> a, sp<Numeric> b, sp<Numeric> t)
{
    return sp<Interpolate<float, float>>::make(std::move(a), std::move(b), std::move(t));
}

sp<Vec2> Math::lerp(sp<Vec2> a, sp<Vec2> b, sp<Numeric> t)
{
    return sp<Interpolate<V2, float>>::make(std::move(a), std::move(b), std::move(t));
}

sp<Vec3> Math::lerp(sp<Vec3> a, sp<Vec3> b, sp<Numeric> t)
{
    return sp<Interpolate<V3, float>>::make(std::move(a), std::move(b), std::move(t));
}

sp<Vec4> Math::lerp(sp<Vec4> a, sp<Vec4> b, sp<Numeric> t)
{
    return sp<Interpolate<V4, float>>::make(std::move(a), std::move(b), std::move(t));
}

sp<Vec2> Math::bezier(sp<Vec2> p0, sp<Vec2> p1, sp<Vec2> p2, sp<Numeric> t)
{
    sp<Vec2> i1 = lerp(std::move(p0), p1, t);
    sp<Vec2> i2 = lerp(std::move(p1), std::move(p2), t);
    return lerp(std::move(i1), std::move(i2), std::move(t));
}

V3 Math::quadratic(float a, float b, float c)
{
    float d = b * b - 4.0f * a * c;
    if(d < 0)
        return V3(d, 0, 0);
    float sqrtd = std::sqrt(d);
    return V3(d, (-b + sqrtd) / 2.0f / a, (-b - sqrtd) / 2.0f / a);
}

V2 Math::projectile(float dx, float dy, float v, float g, uint32_t sid)
{
	float a = 1 + dy * dy / dx / dx;
	float b = -dy * g - v * v;
	float c = g * g * dx * dx * 0.25f;
    const V3 solutions = quadratic(a, b, c);
    DWARN(sid == 0 || sid == 1, "Illegal solution id: %d, should be 0 or 1", sid);
    float vx2 = sid == 0 ? solutions.y() : solutions.z();
    if(solutions.x() < 0 || vx2 < 0)
		return V2(0, 0);
    float vx = std::sqrt(vx2);
    float vy = std::sqrt(v * v - vx2);
    return V2(dx > 0 ? vx : -vx, g < 0 ? vy : -vy);
}

V2 Math::normalize(const V2& v2)
{
    const glm::vec2 n = glm::normalize(glm::vec2(v2.x(), v2.y()));
    return V2(n.x, n.y);
}

V3 Math::normalize(const V3& v3)
{
    const glm::vec3 n = glm::normalize(glm::vec3(v3.x(), v3.y(), v3.z()));
    return V3(n.x, n.y, n.z);
}

V4 Math::normalize(const V4& v4)
{
    const glm::vec4 n = glm::normalize(glm::vec4(v4.x(), v4.y(), v4.z(), v4.w()));
    return V4(n.x, n.y, n.z, n.w);
}

/**
 *
 * s0 = a * sin(t0) + o
 * s1 = a * sin(t1) + o
 * v0 = a * cos(t0)
 * v1 = a * cos(t1)
 *
 * (s0 - o) ^ 2 + v0 ^ 2 = a ^ 2
 * (s1 - o) ^ 2 + v1 ^ 2 = a ^ 2
 *
 * (s0 - s1) * (s0 + s1 - 2 * o) + v0 ^ 2 - v1 ^ 2 = 0
 * s0 + s1 - 2 * o = (v1 ^ 2 - v0 ^ 2) / (s0 - s1)
 * o = (s0 + s1 - (v1 ^ 2 - v0 ^ 2) / (s0 - s1)) / 2
 * a ^ 2 = (s0 - 0) ^ 2  + v0 ^ 2
 *
 */
void Math::vibrate(float s0, float v0, float s1, float v1, float& o, float& a, float& t0, float& t1)
{
    if(s0 == s1)
    {
        t0 = t1 = 0;
        o = s0;
        a = 1.0f;
        return;
    }

    DCHECK(s0 != s1, "Same vibration point %.2f %.2f", s0, s1);
    o = (s0 + s1 - (v1 * v1 - v0 * v0) / (s0 - s1)) / 2.0f;
    a = sqrt((s0 - o) * (s0 - o) + v0 * v0);
    t0 = acos(std::min(1.0f, std::abs(v0 / a)));
    t1 = acos(std::min(1.0f, std::abs(v1 / a)));

    float sx0 = a * sin(t0);
    if(std::abs(sx0 - s0 + o) > std::abs(-sx0 - s0 + o))
        t0 = -t0;
    float sx1 = a * sin(t1);
    if(std::abs(sx1 - s1 + o) > std::abs(-sx1 - s1 + o))
        t1 = -t1;

    if(t0 == t1)
        t0 -= (v0 == 0 ? PI : PIx2);
}

}
