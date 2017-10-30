#include "graphics/base/bounds.h"

#include <vector>

#include "core/base/bean_factory.h"
#include "core/util/bean_utils.h"

#include "graphics/base/rect.h"
#include "graphics/base/size.h"
#include "graphics/base/vec2.h"

namespace ark {

namespace {

std::vector<String> psplit(const String& value)
{
    std::vector<String> strs;
    uint32_t pos = 0;
    for(uint32_t i = 0; i < value.length(); ++i)
    {
        char c = value.at(i);
        if(c == ',')
        {
            strs.push_back(value.substr(pos, i));
            pos = i + 1;
        }
        else if(c == '(')
            i = Strings::parentheses(value, i);
    }
    if(pos != value.length())
        strs.push_back(value.substr(pos).strip());
    return strs;
}

}

Bounds::Bounds()
    : _position(Null::ptr<Vec2>()), _size(Null::ptr<Size>())
{
}

Bounds::Bounds(const Rect& rectf)
    : _position(sp<Vec2>::make(rectf.left(), rectf.top())), _size(sp<Size>::make(rectf.width(), rectf.height()))
{
}

Bounds::Bounds(float x, float y, float w, float h)
    : _position(sp<Vec2>::make(x, y)), _size(sp<Size>::make(w, h))
{
}

Bounds::Bounds(const sp<Vec2>& position, const sp<Size>& size)
    : _position(position), _size(size)
{
}

bool Bounds::ptin(float x, float y) const
{
    Rect rectf(_position->x(), _position->y(), _position->x() + _size->width(), _position->y() + _size->height());
    return rectf.ptin(x, y);
}

const sp<Vec2>& Bounds::position() const
{
    return _position;
}

float Bounds::left() const
{
    return _position->x();
}

float Bounds::top() const
{
    return _position->y();
}

float Bounds::right() const
{
    return _position->x() + _size->width();
}

float Bounds::bottom() const
{
    return _position->y() + _size->height();
}

float Bounds::width() const
{
    return _size->width();
}

float Bounds::height() const
{
    return _size->height();
}

const sp<Size>& Bounds::size()
{
    return _size;
}

Bounds::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
{
    std::vector<String> strs = psplit(value);
    if(strs.size() == 1)
        strs = psplit(Strings::unwrap(strs[0], '(', ')'));

    if(strs.size() == 2)
    {
        _position = parent.ensureBuilder<Vec2>(strs[0]);
        _size = parent.ensureBuilder<Size>(strs[1]);
    }
    else if(strs.size() == 4)
    {
        _x = parent.ensureBuilder<Numeric>(strs[0]);
        _y = parent.ensureBuilder<Numeric>(strs[1]);
        _w = parent.ensureBuilder<Numeric>(strs[2]);
        _h = parent.ensureBuilder<Numeric>(strs[3]);
    }
    else
        DFATAL("Illegal bounds expression: %s", value.c_str());
}

sp<Bounds> Bounds::DICTIONARY::build(const sp<Scope>& args)
{
    if(_position && _size)
        return sp<Bounds>::make(_position->build(args), _size->build(args));
    return sp<Bounds>::make(BeanUtils::toFloat(_x, args), BeanUtils::toFloat(_y, args), BeanUtils::toFloat(_w, args), BeanUtils::toFloat(_h, args));
}

}
