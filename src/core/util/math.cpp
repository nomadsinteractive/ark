#include "core/util/math.h"

#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <math_neon.h>

#include "core/util/log.h"

namespace ark {

const float Math::PI = static_cast<float>(M_PI);
const float Math::PIx2 = static_cast<float>(M_PI * 2.0f);
const float Math::PI_2 = static_cast<float>(M_PI_2);
const float Math::PI_4 = static_cast<float>(M_PI_4);

uint32_t Math::log2(uint32_t value)
{
    static const int tab32[32] = {
        0,  9,  1, 10, 13, 21,  2, 29,
       11, 14, 16, 18, 22, 25,  3, 30,
        8, 12, 20, 28, 15, 17, 24,  7,
       19, 27, 23,  6, 26,  5,  4, 31};
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return tab32[(uint32_t) (value * 0x07C4ACDD) >> 27];
}

float Math::sin(float x)
{
    return sinf_neon(x);
}

float Math::cos(float x)
{
    return cosf_neon(x);
}

float Math::acos(float x)
{
    return acosf_neon(x);
}

float Math::atan2(float y, float x)
{
    return atan2f_neon(y, x);
}

int32_t Math::floor(float x)
{
    return (int32_t) x;
}

int32_t Math::round(float x)
{
    return (int32_t) (x + .5f);
}

float Math::randf()
{
    return rand() / static_cast<float>(RAND_MAX);
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
    return sqrtf_neon(x);
}

float Math::rsqrt(float number)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = number * 0.5f;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

V3 Math::quadratic(float a, float b, float c)
{
	float d = b * b - 4.0f * a * c;
	if(d < 0)
		return V3(d, 0, 0);
	float sqrtd = ::sqrt(d);
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
	float vx = ::sqrt(vx2);
	float vy = ::sqrt(v * v - vx2);
	return V2(dx > 0 ? vx : -vx, g < 0 ? vy : -vy);
}

}
