#ifndef ARK_APP_BASE_EVENT_DISPATCHER_H_
#define ARK_APP_BASE_EVENT_DISPATCHER_H_

#include <map>
#include <stack>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "app/base/event.h"
#include "app/inf/event_listener.h"

namespace ark {

class ARK_API EventDispatcher : public EventListener {
public:
//  [[script::bindings::auto]]
    EventDispatcher();

//  [[script::bindings::auto]]
    void onKeyEvent(Event::Code code, sp<Runnable> onPress, sp<Runnable> onRelease, sp<Runnable> onRepeat);
//  [[script::bindings::auto]]
    void unKeyEvent(Event::Code code);

//  [[script::bindings::auto]]
    void onMotionEvent(sp<EventListener> onDown, sp<EventListener> onUp, sp<EventListener> onClick, sp<EventListener> onMove);
//  [[script::bindings::auto]]
    void unMotionEvent();

    float motionClickRange() const;

//  [[script::bindings::auto]]
    virtual bool onEvent(const Event& event) override;

private:
    class KeyEventListener {
    public:
        KeyEventListener(sp<Runnable> onPress, sp<Runnable> onRelease, sp<Runnable> onRepeat);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(KeyEventListener);

        void onEvent(const EventDispatcher& dispatcher, const Event& event);

    private:
        sp<Runnable> _on_press;
        sp<Runnable> _on_release;
        sp<Runnable> _on_repeat;
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
    float _motion_click_range;

    std::map<Event::Code, std::stack<KeyEventListener>> _key_events;
    std::stack<MotionEventListener> _motion_events;
};

}

#endif
