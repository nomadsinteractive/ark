#include "graphics/base/color.h"

#include <algorithm>

#include "core/util/strings.h"

namespace ark {

const Color Color::NONE(0);
const Color Color::WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const Color Color::BLACK(0.0f, 0.0f, 0.0f, 1.0f);

Color::Color()
    : _color(0, 0, 0, 0)
{
}

Color::Color(uint32_t value)
    : _color((value >> 24) / 255.0f, ((value >> 16) & 0xff) / 255.0f, ((value >> 8) & 0xff) / 255.0f, (value & 0xff) / 255.0f)
{
}

Color::Color(float red, float green, float blue, float alpha)
    : _color(red, green, blue, alpha)
{
}

Color::Color(float red, float green, float blue)
    : _color(red, green, blue, 1.0f)
{
}

Color::Color(const Color& color)
    : _color(color._color)
{
}

float Color::red() const
{
    return _color._x;
}

void Color::setRed(float red)
{
    _color._x = red;
    _changed = true;
}

float Color::green() const
{
    return _color._y;
}

void Color::setGreen(float green)
{
    _color._y = green;
    _changed = true;
}

float Color::blue() const
{
    return _color._z;
}

void Color::setBlue(float blue)
{
    _color._z = blue;
    _changed = true;
}

float Color::alpha() const
{
    return _color.w();
}

void Color::setAlpha(float alpha)
{
    _color._w = alpha;
    _changed = true;
}

uint32_t Color::value() const
{
    uint8_t v[4];
    v[3] = static_cast<uint8_t>(_color.x() * 255);
    v[2] = static_cast<uint8_t>(_color.y() * 255);
    v[1] = static_cast<uint8_t>(_color.z() * 255);
    v[0] = static_cast<uint8_t>(_color.w() * 255);
    return *reinterpret_cast<uint32_t*>(v);
}

void Color::setValue(uint32_t value)
{
    _color = V4((value >> 24) / 255.0f, ((value >> 16) & 0xff) / 255.0f, ((value >> 8) & 0xff) / 255.0f, (value & 0xff) / 255.0f);
    _changed = true;
}

void Color::assign(const Color& other)
{
    _color = other._color;
    _changed = true;
}

bool Color::operator ==(const Color& other) const
{
    return _color == other._color;
}

bool Color::operator !=(const Color& other) const
{
    return _color != other._color;
}

const Color& Color::operator =(const Color& other)
{
    _color = other._color;
    return *this;
}

V4 Color::val()
{
    return _color;
}

template<> ARK_API Color Conversions::to<String, Color>(const String& s)
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
