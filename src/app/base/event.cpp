#include "app/base/event.h"

namespace ark {

Event::Event(Event::Action action, float x, float y, uint32_t timestamp, Code code)
    : _action(action), _x(x), _y(y), _timestamp(timestamp), _code(code)
{
}

Event::Event(Event::Action action, int32_t x, int32_t y, uint32_t timestamp, Code code)
    : _action(action), _x(static_cast<float>(x)), _y(static_cast<float>(y)), _timestamp(timestamp), _code(code)
{
}

Event::Event(Event::Action action, uint32_t timestamp, Event::Code code)
    : _action(action), _x(0.0f), _y(0.0f), _timestamp(timestamp), _code(code)
{
}

Event::Event(const Event& other)
    : _action(other._action), _x(other._x), _y(other._y), _timestamp(other._timestamp), _code(other._code)
{
}

bool Event::ptin(const Rect& rectf) const
{
    return rectf.ptin(_x, _y);
}

Event::Action Event::action() const
{
    return _action;
}

float Event::x() const
{
    return _x;
}

float Event::y() const
{
    return _y;
}

V2 Event::xy() const
{
    return V2(_x, _y);
}

uint32_t Event::timestamp() const
{
    return _timestamp;
}

Event::Code Event::code() const
{
    return _code;
}

wchar_t Event::toCharacter(Event::Code code)
{
    return static_cast<wchar_t>(code);
}

}
