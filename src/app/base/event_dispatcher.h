#ifndef ARK_APP_BASE_EVENT_DISPATCHER_H_
#define ARK_APP_BASE_EVENT_DISPATCHER_H_

#include <map>
#include <stack>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "app/base/event.h"
#include "app/inf/event_listener.h"

namespace ark {

//[[core::class]]
class ARK_API EventDispatcher : public EventListener {
public:
//  [[script::bindings::auto]]
    EventDispatcher();

//  [[script::bindings::auto]]
    void onKeyEvent(Event::Code code, const sp<Runnable>& onPress, const sp<Runnable>& onRelease, const sp<Runnable>& onClick, const sp<Runnable>& onRepeat);
//  [[script::bindings::auto]]
    void unKeyEvent(Event::Code code);

//  [[script::bindings::auto]]
    void onMotionEvent(const sp<EventListener>& onDown, const sp<EventListener>& onUp, const sp<EventListener>& onClick, const sp<EventListener>& onMove);
//  [[script::bindings::auto]]
    void unMotionEvent();

    uint32_t keyClickInterval() const;
    float motionClickRange() const;

//  [[script::bindings::auto]]
    virtual bool onEvent(const Event& event) override;

private:
    class KeyEventListener {
    public:
        KeyEventListener(const sp<Runnable>& onPress, const sp<Runnable>& onRelease, const sp<Runnable>& onClick, const sp<Runnable>& onRepeat);
        KeyEventListener(const KeyEventListener& other) = default;
        KeyEventListener(KeyEventListener&& other) = default;

        void onEvent(const EventDispatcher& dispatcher, const Event& event);

    private:
        sp<Runnable> _on_press;
        sp<Runnable> _on_release;
        sp<Runnable> _on_click;
        sp<Runnable> _on_repeat;

        uint32_t _pressed_timestamp;
    };

    class MotionEventListener {
    public:
        MotionEventListener(const sp<EventListener>& onPress, const sp<EventListener>& onRelease, const sp<EventListener>& onClick, const sp<EventListener>& onMove);
        MotionEventListener(const MotionEventListener& other) = default;
        MotionEventListener(MotionEventListener&& other) = default;

        MotionEventListener& operator =(const MotionEventListener& other) = default;
        MotionEventListener& operator =(MotionEventListener&& other) = default;

        bool onEvent(const EventDispatcher& dispatcher, const Event& event);

    private:
        sp<EventListener> _on_down;
        sp<EventListener> _on_up;
        sp<EventListener> _on_click;
        sp<EventListener> _on_move;

        float _pressed_x;
        float _pressed_y;
    };

private:
    uint32_t _key_click_interval;
    float _motion_click_range;

    std::map<Event::Code, std::stack<KeyEventListener>> _key_events;
    std::stack<MotionEventListener> _motion_events;
};

}

#endif
