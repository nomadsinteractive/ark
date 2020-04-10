#include "app/base/event.h"

namespace ark {

Event::Event(Event::Action action, uint32_t timestamp, const EventInfo& info)
    : _action(action), _timestamp(timestamp), _info(info)
{
}

Event::Event(const Event& other, float x, float y)
    : _action(other._action), _timestamp(other._timestamp), _info(other._info)
{
    _info._button._x = x;
    _info._button._y = y;
}

bool Event::ptin(const Rect& rectf) const
{
    return rectf.ptin(x(), y());
}

Event::Action Event::action() const
{
    return _action;
}

float Event::x() const
{
    return _info._button._x;
}

float Event::y() const
{
    return _info._button._y;
}

V2 Event::xy() const
{
    return V2(_info._button._x, _info._button._y);
}

uint32_t Event::timestamp() const
{
    return _timestamp;
}

Event::Code Event::code() const
{
    return _info._code;
}

Event::Button Event::button() const
{
    return _info._button._which;
}

wchar_t Event::toCharacter() const
{
    return static_cast<wchar_t>(_info._code);
}

Event::EventInfo::EventInfo(Event::Code code)
    : _code(code)
{
}

Event::EventInfo::EventInfo(const Event::ButtonInfo& button)
    : _button(button)
{
}

Event::EventInfo::EventInfo(const Event::MotionInfo& motion)
    : _motion(motion)
{
}

Event::ButtonInfo::ButtonInfo(float x, float y, Event::Button which)
    : _x(x), _y(y), _which(which)
{
}

Event::MotionInfo::MotionInfo(float x, float y, Event::Button which, uint32_t states)
    : _x(x), _y(y), _which(which), _states(states)
{
}

}
