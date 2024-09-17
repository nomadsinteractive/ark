#include "app/base/event.h"

namespace ark {

Event::Event(Event::Action action, uint64_t timestamp, const EventInfo& info)
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

V2 Event::xyFragCoord() const
{
    return _info._button._xy_frag_coord;
}

uint64_t Event::timestamp() const
{
    return _timestamp;
}

Event::Code Event::code() const
{
    return _info._keyboard._code;
}

Event::Button Event::button() const
{
    return _info._button._which;
}

wchar_t Event::character() const
{
    return _info._keyboard._character;
}

const char* Event::textInput() const
{
    return _info._text_input._text;
}

Event::TextInputInfo::TextInputInfo(const String& text)
{
    std::strncpy(_text, text.c_str(), sizeof(_text));
}

Event::KeyboardInfo::KeyboardInfo(Code code, wchar_t character)
    : _code(code), _character(character)
{
}

Event::EventInfo::EventInfo(const TextInputInfo& textInput)
    : _text_input(textInput)
{
}

Event::EventInfo::EventInfo(const KeyboardInfo& keyboard)
    : _keyboard(keyboard)
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

}
