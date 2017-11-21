#include "app/base/event_dispatcher.h"

#include "core/inf/runnable.h"
#include "core/util/math.h"

namespace ark {

EventDispatcher::EventDispatcher()
    : _key_click_interval(500), _motion_click_range(50.0f)
{
}

void EventDispatcher::onKeyEvent(Event::Code code, const sp<Runnable>& onPress, const sp<Runnable>& onRelease, const sp<Runnable>& onClick, const sp<Runnable>& onRepeat)
{
    _key_events[code].emplace(onPress, onRelease, onClick, onRepeat);
}

void EventDispatcher::unKeyEvent(Event::Code code)
{
    auto iter = _key_events.find(code);
    if(iter != _key_events.end())
        if(!iter->second.empty())
            iter->second.pop();
}

void EventDispatcher::onMotionEvent(const sp<EventListener>& onDown, const sp<EventListener>& onUp, const sp<EventListener>& onClick, const sp<EventListener>& onMove)
{
    _motion_events.emplace(onDown, onUp, onClick, onMove);
}

void EventDispatcher::unMotionEvent()
{
    if(!_motion_events.empty())
        _motion_events.pop();
}

uint32_t EventDispatcher::keyClickInterval() const
{
    return _key_click_interval;
}

float EventDispatcher::motionClickRange() const
{
    return _motion_click_range;
}

bool EventDispatcher::onEvent(const Event& event)
{
    Event::Action action = event.action();
    if(action == Event::ACTION_KEY_DOWN || action == Event::ACTION_KEY_UP || action == Event::ACTION_KEY_REPEAT)
    {
        const auto iter = _key_events.find(event.code());
        if(iter != _key_events.end() && !iter->second.empty())
            iter->second.top().onEvent(*this, event);
        return true;
    }
    else if(action == Event::ACTION_DOWN || action == Event::ACTION_UP || action == Event::ACTION_MOVE || action == Event::ACTION_CANCEL)
    {
        if(!_motion_events.empty())
            return _motion_events.top().onEvent(*this, event);
    }
    return false;
}

EventDispatcher::KeyEventListener::KeyEventListener(const sp<Runnable>& onPress, const sp<Runnable>& onRelease, const sp<Runnable>& onClick, const sp<Runnable>& onRepeat)
    : _on_press(onPress), _on_release(onRelease), _on_click(onClick), _on_repeat(onRepeat), _pressed_timestamp(0)
{
}

void EventDispatcher::KeyEventListener::onEvent(const EventDispatcher& dispatcher, const Event& event)
{
    Event::Action action = event.action();
    if(action == Event::ACTION_KEY_DOWN)
    {
        _pressed_timestamp = event.timestamp();
        if(_on_press)
            _on_press->run();
    }
    else if(action == Event::ACTION_KEY_UP)
    {
        uint32_t d = event.timestamp() - _pressed_timestamp;
        _pressed_timestamp = 0;
        if(_on_click && d < dispatcher.keyClickInterval())
            _on_click->run();
        if(_on_release)
            _on_release->run();
    }
    else if(action == Event::ACTION_KEY_REPEAT)
    {
        if(_on_repeat)
            _on_repeat->run();
    }
    else
        DWARN(false, "Unknown event: %d", event.action());
}

EventDispatcher::MotionEventListener::MotionEventListener(const sp<EventListener>& onPress, const sp<EventListener>& onRelease, const sp<EventListener>& onClick, const sp<EventListener>& onMove)
    : _on_down(onPress), _on_up(onRelease), _on_click(onClick), _on_move(onMove), _pressed_x(0), _pressed_y(0)
{
}

bool EventDispatcher::MotionEventListener::onEvent(const EventDispatcher& dispatcher, const Event& event)
{
    Event::Action action = event.action();
    if(action == Event::ACTION_DOWN)
    {
        _pressed_x = event.x();
        _pressed_y = event.y();
        if(_on_down)
            return _on_down->onEvent(event);
    }
    else if(action == Event::ACTION_UP)
    {
        if(_on_click && Math::hypot(event.x() - _pressed_x, event.y() - _pressed_y) < dispatcher.motionClickRange())
            return _on_click->onEvent(event);
        if(_on_up)
            _on_up->onEvent(event);
    }
    else if(action == Event::ACTION_MOVE)
    {
        if(_on_move)
            return _on_move->onEvent(event);
    }
    else if(action == Event::ACTION_CANCEL)
        _pressed_y = _pressed_x = -1000.0f;
    else
        DWARN(false, "Unknown event: %d", event.action());
    return false;
}

}
