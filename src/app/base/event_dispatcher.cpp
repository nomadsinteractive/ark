#include "app/base/event_dispatcher.h"

#include "core/inf/runnable.h"
#include "core/util/math.h"

namespace ark {

EventDispatcher::EventDispatcher()
    : _motion_click_range(50.0f)
{
}

void EventDispatcher::onKeyEvent(Event::Code code, sp<Runnable> onPress, sp<Runnable> onRelease, sp<Runnable> onRepeat)
{
    _key_events[code].emplace(std::move(onPress), std::move(onRelease), std::move(onRepeat));
}

void EventDispatcher::unKeyEvent(Event::Code code)
{
    if(const auto iter = _key_events.find(code); iter != _key_events.end())
        if(!iter->second.empty())
            iter->second.pop();
}

void EventDispatcher::onMotionEvent(sp<EventListener> onPress, sp<EventListener> onRelease, sp<EventListener> onClick, sp<EventListener> onMove)
{
    _motion_events.emplace(std::move(onPress), std::move(onRelease), std::move(onClick), std::move(onMove));
}

void EventDispatcher::unMotionEvent()
{
    if(!_motion_events.empty())
        _motion_events.pop();
}

float EventDispatcher::motionClickRange() const
{
    return _motion_click_range;
}

bool EventDispatcher::onEvent(const Event& event)
{
    if(const Event::Action action = event.action(); action == Event::ACTION_KEY_DOWN || action == Event::ACTION_KEY_UP || action == Event::ACTION_KEY_REPEAT)
    {
        if(const auto iter = _key_events.find(event.code()); iter != _key_events.end() && !iter->second.empty())
        {
            iter->second.top().onEvent(*this, event);
            return true;
        }
    }
    else if(action == Event::ACTION_DOWN || action == Event::ACTION_UP || action == Event::ACTION_MOVE || action == Event::ACTION_CANCEL)
    {
        if(!_motion_events.empty())
            return _motion_events.top().onEvent(*this, event);
    }
    return false;
}

EventDispatcher::KeyEventListener::KeyEventListener(sp<Runnable> onPress, sp<Runnable> onRelease, sp<Runnable> onRepeat)
    : _on_press(std::move(onPress)), _on_release(std::move(onRelease)), _on_repeat(std::move(onRepeat))
{
}

void EventDispatcher::KeyEventListener::onEvent(const EventDispatcher& dispatcher, const Event& event) const
{
    if(const Event::Action action = event.action(); action == Event::ACTION_KEY_DOWN)
    {
        if(_on_press)
            _on_press->run();
    }
    else if(action == Event::ACTION_KEY_UP)
    {
        if(_on_release)
            _on_release->run();
    }
    else if(action == Event::ACTION_KEY_REPEAT)
    {
        if(_on_repeat)
            _on_repeat->run();
    }
    else
        DCHECK_WARN(false, "Unknown event: %d", event.action());
}

EventDispatcher::MotionEventListener::MotionEventListener(const sp<EventListener>& onPress, const sp<EventListener>& onRelease, const sp<EventListener>& onClick, const sp<EventListener>& onMove)
    : _on_press(onPress), _on_release(onRelease), _on_click(onClick), _on_move(onMove), _pressed_x(0), _pressed_y(0)
{
}

bool EventDispatcher::MotionEventListener::onEvent(const EventDispatcher& dispatcher, const Event& event)
{
    if(const Event::Action action = event.action(); action == Event::ACTION_DOWN)
    {
        _pressed_x = event.x();
        _pressed_y = event.y();
        if(_on_press)
            return _on_press->onEvent(event);
    }
    else if(action == Event::ACTION_UP)
    {
        if(_on_click && Math::hypot(event.x() - _pressed_x, event.y() - _pressed_y) < dispatcher.motionClickRange())
        {
            if(_on_click->onEvent(event))
                return true;
        }
        if(_on_release)
            _on_release->onEvent(event);
    }
    else if(action == Event::ACTION_MOVE)
    {
        if(_on_move)
            return _on_move->onEvent(event);
    }
    else if(action == Event::ACTION_CANCEL)
        _pressed_y = _pressed_x = -1000.0f;
    else
        DCHECK_WARN(false, "Unknown event: %d", event.action());
    return false;
}

}
