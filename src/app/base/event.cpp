#include "app/base/event.h"

namespace ark {

Event::Event(Event::Action action, uint32_t timestamp, const EventInfo& info)
    : _action(action), _timestamp(timestamp), _info(info)
{
}

Event::Event(const Event& other, const V2& xy)
    : _action(other._action), _timestamp(other._timestamp), _info(other._info)
{
    _info._button._xy = xy;
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
    return _info._button._xy.x();
}

float Event::y() const
{
    return _info._button._xy.y();
}

V2 Event::xy() const
{
    return _info._button._xy;
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

Event::ButtonInfo::ButtonInfo(const V2& xy, Event::Button which)
    : _xy(xy), _which(which)
{
}

Event::MotionInfo::MotionInfo(const V2& xy, Event::Button which, uint32_t states)
    : _xy(xy), _which(which), _states(states)
{
}

}
