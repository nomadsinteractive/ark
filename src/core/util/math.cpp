#include "core/util/math.h"

#include <algorithm>
#include <math.h>
#include <stdlib.h>

namespace ark {

const float Math::PI = 3.14159265358979f;
const float Math::PIx2 = 3.14159265358979f * 2.0f;
const float Math::PI_2 = 3.14159265358979f / 2.0f;
const float Math::PI_4 = 3.14159265358979f / 4.0f;

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
    if(x > PI)
        return sin(x - floor((x + PI) / PIx2) * PIx2);
    else if(x < -PI)
        return sin(x - floor((x - PI) / PIx2) * PIx2);

    const float B = 4 / PI;
    const float C = -4 / (PI * PI);
    const float P = 0.225f;

    float y = B * x + C * x * abs(x);
    y = P * (y * abs(y) - y) + y;
    return y;
}

float Math::cos(float x)
{
    return sin(x + PI_2);
}

float Math::acos(float x)
{
    float negate = x < 0 ? 1.0f : 0.0f;
    float ret = -0.0187293f;
    x = Math::abs<float>(x);
    ret = ret * x;
    ret = ret + 0.0742610f;
    ret = ret * x;
    ret = ret - 0.2121144f;
    ret = ret * x;
    ret = ret + 1.5707288f;
    ret = ret * Math::sqrt(1.0f - x);
    ret = ret - 2 * negate * ret;
    return negate * PI + ret;
}

// https://gist.github.com/volkansalma/2972237
// vs
// https://math.stackexchange.com/questions/1098487/atan2-faster-approximation
float Math::atan2(float y, float x)
{
	if(x == 0.0f)
	{
		if (y > 0.0f) return PI_2;
		if ( y == 0.0f ) return 0.0f;
		return -PI_2;
	}
/*
	float atan;
	float z = y / x;
    float absz = Math::abs<float>(z);
	if(absz < 1.0f)
		return PI_4 * z - z * (absz - 1.0f) * (0.2447f + 0.0663f * absz);
	atan = PI_2 - z / (z * z + 0.28f);
	if (y < 0.0f ) return atan - PI;
	return atan;
/*/
	float ax = abs(x), ay = abs(y);
	float a = std::min(ax, ay) / std::max(ax, ay);
	float s = a * a;
	float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;
	if (ay > ax) r = PI_2 - r;
	if (x < 0) r = PI - r;
	if (y < 0) r = -r;
	return r;
/**/
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

    uint32_t i = *(uint32_t*) &x;
    // adjust bias
    i  += 127 << 23;
    // approximation of square root
    i >>= 1;
    return *(float*) &i;
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
