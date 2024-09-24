#include "graphics/base/color.h"

#include <algorithm>

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/strings.h"

#include "graphics/impl/vec/vec4_impl.h"
#include "graphics/util/vec3_type.h"

namespace ark {

Color::Color()
    : Color(0, 0, 0, 0)
{
}

Color::Color(uint32_t value)
    : Color((value >> 24) / 255.0f, ((value >> 16) & 0xff) / 255.0f, ((value >> 8) & 0xff) / 255.0f, (value & 0xff) / 255.0f)
{
}

Color::Color(float red, float green, float blue, float alpha)
    : _wrapped(sp<Vec4Impl>::make(red, green, blue, alpha))
{
}

sp<Numeric> Color::r() const
{
    return _wrapped->x();
}

void Color::setR(float red)
{
    _wrapped->x()->set(red);
}

sp<Numeric> Color::g() const
{
    return _wrapped->y();
}

void Color::setG(float green)
{
    _wrapped->y()->set(green);
}

sp<Numeric> Color::b() const
{
    return _wrapped->z();
}

void Color::setB(float blue)
{
    _wrapped->z()->set(blue);
}

sp<Numeric> Color::a() const
{
    return _wrapped->w();
}

void Color::setA(float alpha)
{
    _wrapped->w()->set(alpha);
}

V4 Color::rgba() const
{
    return _wrapped->val();
}

sp<Vec3> Color::toVec3() const
{
    return Vec3Type::create(_wrapped->x(), _wrapped->y(), _wrapped->z());
}

uint32_t Color::value() const
{
    const V4 color = rgba();
    uint8_t v[4];
    v[3] = static_cast<uint8_t>(color.x() * 255);
    v[2] = static_cast<uint8_t>(color.y() * 255);
    v[1] = static_cast<uint8_t>(color.z() * 255);
    v[0] = static_cast<uint8_t>(color.w() * 255);
    return *reinterpret_cast<uint32_t*>(v);
}

void Color::reset(uint32_t value)
{
    setR((value >> 24) / 255.0f);
    setG(((value >> 16) & 0xff) / 255.0f);
    setB(((value >> 8) & 0xff) / 255.0f);
    setA((value & 0xff) / 255.0f);
}

bool Color::operator ==(const Color& other) const
{
    return _wrapped->val() == other._wrapped->val();
}

bool Color::operator !=(const Color& other) const
{
    return _wrapped->val() != other._wrapped->val();
}

V4 Color::val()
{
    return _wrapped->val();
}

bool Color::update(uint64_t timestamp)
{
    return _wrapped->update(timestamp);
}

const sp<Vec4Impl>& Color::wrapped() const
{
    return _wrapped;
}

template<> ARK_API Color StringConvert::eval<Color>(const String& s)
{
    const String val = s.toLower();
    uint32_t r = 0, g = 0, b = 0, a = 255;
    if(val.length() < 8)
        sscanf(val.c_str(), "#%02x%02x%02x", &r, &g, &b);
    else
        sscanf(val.c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a);
    return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

}
