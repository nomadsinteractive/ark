#include "graphics/base/rect.h"

#include "core/inf/array.h"
#include "core/types/shared_ptr.h"
#include "core/util/conversions.h"
#include "core/util/dictionaries.h"
#include "core/util/documents.h"

#include "graphics/base/v2.h"

namespace ark {

Rect::Rect(const Rect& other) noexcept
    : _left(other._left), _top(other._top), _right(other._right), _bottom(other._bottom)
{
}

Rect::Rect(float left, float top, float right, float bottom) noexcept
    : _left(left), _top(top), _right(right), _bottom(bottom)
{
}

float Rect::left() const
{
    return _left;
}

void Rect::setLeft(float left)
{
    _left = left;
}

float Rect::top() const
{
    return _top;
}

void Rect::setTop(float top)
{
    _top = top;
}

float Rect::right() const
{
    return _right;
}

void Rect::setRight(float right)
{
    _right = right;
}

float Rect::bottom() const
{
    return _bottom;
}

void Rect::setBottom(float bottom)
{
    _bottom = bottom;
}

float Rect::width() const
{
    return std::abs(_right - _left);
}

float Rect::height() const
{
    return std::abs(_bottom - _top);
}

void Rect::scale(float xScale, float yScale)
{
    _left = _left * xScale;
    _right = _right * xScale;
    _top = _top * yScale;
    _bottom = _bottom * yScale;
}

void Rect::scale(const V2& s)
{
    scale(s.x(), s.y());
}

void Rect::translate(float x, float y)
{
    _left += x;
    _right += x;
    _top += y;
    _bottom += y;
}

void Rect::vflip(float height)
{
    std::swap(_top, _bottom);
    _top = height - _top;
    _bottom = height - _bottom;
}

bool Rect::ptin(float x, float y) const
{
    if(_left <= _right && (x < _left || x > _right))
        return false;
    if(_left > _right && (x > _left || x < _right))
        return false;
    if(_top <= _bottom && (y < _top || y > _bottom))
        return false;
    if(_top > _bottom && (y > _top || y < _bottom))
        return false;
    return true;
}

bool Rect::overlap(const Rect& other) const
{
    return _left < other._right && _right > other._left && _top < other._bottom && _bottom > other._top;
}

void Rect::setCenter(float x, float y)
{
    float dx = x - (_left + _right) / 2;
    float dy = y - (_top + _bottom) / 2;
    _left += dx;
    _right += dx;
    _top += dy;
    _bottom += dy;
}

Rect Rect::parse(const std::map<String, String>& properties)
{
    float left = Strings::parse<float>(Strings::getProperty(properties, Constants::Attributes::LEFT));
    float top = Strings::parse<float>(Strings::getProperty(properties, Constants::Attributes::TOP));
    const String& rightStr = Strings::getProperty(properties, Constants::Attributes::RIGHT);
    const String& bottomStr = Strings::getProperty(properties, Constants::Attributes::BOTTOM);
    float right = rightStr.empty() ? left + Strings::getProperty<float>(properties, Constants::Attributes::WIDTH) : Strings::parse<float>(rightStr);
    float bottom = bottomStr.empty() ? top + Strings::getProperty<float>(properties, Constants::Attributes::HEIGHT) : Strings::parse<float>(bottomStr);
    return Rect(left, top, right, bottom);
}

Rect Rect::parse(const document& manifest)
{
    const String rect = Documents::getAttribute(manifest, Constants::Attributes::RECT);
    if(rect)
        return Strings::parse<Rect>(rect);

    const Rect bounds = Strings::parse<Rect>(Documents::ensureAttribute(manifest, Constants::Attributes::BOUNDS));
    return Rect(bounds.left(), bounds.top(), bounds.left() + bounds.right(), bounds.top() + bounds.bottom());
}

template<> ARK_API Rect Conversions::to<String, Rect>(const String& str)
{
    const std::vector<String> m = Strings::unwrap(str.strip(), '(', ')').split(',');
    if(m.size() == 4)
    {
        float left = Strings::parse<float>(m[0]);
        float top = Strings::parse<float>(m[1]);
        float right = Strings::parse<float>(m[2]);
        float bottom = Strings::parse<float>(m[3]);
        return Rect(left, top, right, bottom);
    }
    const std::map<String, String> properties = Strings::parseProperties(str);
    return Rect::parse(properties);
}

template<> ARK_API String Conversions::to<Rect, String>(const Rect& rect)
{
    return Strings::sprintf("(%.2f, %.2f, %.2f, %.2f)", rect.left(), rect.top(), rect.right(), rect.bottom());
}

template<> ARK_API Rect Dictionaries::get<Rect>(BeanFactory& beanFactory, const String& value, const sp<Scope>& args)
{
    const String unwrapped = Strings::unwrap(value.strip(), '(', ')');
    DCHECK(unwrapped, "Empty RectF value");
    std::vector<String> s = unwrapped.split(',');
    DCHECK(s.size() == 4, "Invaild rect value \"%s\"", value.c_str());
    return Rect(Dictionaries::get<float>(beanFactory, s[0], args),
            Dictionaries::get<float>(beanFactory, s[1], args),
            Dictionaries::get<float>(beanFactory, s[2], args),
            Dictionaries::get<float>(beanFactory, s[3], args));
}

}
