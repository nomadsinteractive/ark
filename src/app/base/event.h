#ifndef ARK_APP_BASE_EVENT_H_
#define ARK_APP_BASE_EVENT_H_

#include "core/base/api.h"

#include "graphics/base/rect.h"
#include "graphics/base/v2.h"

namespace ark {

class ARK_API Event {
public:
//  [[script::bindings::enumeration]]
    enum Action {
        ACTION_KEY_NONE,
        ACTION_KEY_DOWN,
        ACTION_KEY_UP,
        ACTION_KEY_REPEAT,
        ACTION_DOWN,
        ACTION_UP,
        ACTION_MOVE,
        ACTION_WHEEL,
        ACTION_BACK_PRESSED,
        ACTION_CANCEL,
        ACTION_USER_DEFINED = 10000
    };
//  [[script::bindings::enumeration]]
    enum Code {
        CODE_NONE,
        CODE_KEYBOARD_A = 'a',
        CODE_KEYBOARD_B,
        CODE_KEYBOARD_C,
        CODE_KEYBOARD_D,
        CODE_KEYBOARD_E,
        CODE_KEYBOARD_F,
        CODE_KEYBOARD_G,
        CODE_KEYBOARD_H,
        CODE_KEYBOARD_I,
        CODE_KEYBOARD_J,
        CODE_KEYBOARD_K,
        CODE_KEYBOARD_L,
        CODE_KEYBOARD_M,
        CODE_KEYBOARD_N,
        CODE_KEYBOARD_O,
        CODE_KEYBOARD_P,
        CODE_KEYBOARD_Q,
        CODE_KEYBOARD_R,
        CODE_KEYBOARD_S,
        CODE_KEYBOARD_T,
        CODE_KEYBOARD_U,
        CODE_KEYBOARD_V,
        CODE_KEYBOARD_W,
        CODE_KEYBOARD_X,
        CODE_KEYBOARD_Y,
        CODE_KEYBOARD_Z,
        CODE_KEYBOARD_0 = '0',
        CODE_KEYBOARD_1,
        CODE_KEYBOARD_2,
        CODE_KEYBOARD_3,
        CODE_KEYBOARD_4,
        CODE_KEYBOARD_5,
        CODE_KEYBOARD_6,
        CODE_KEYBOARD_7,
        CODE_KEYBOARD_8,
        CODE_KEYBOARD_9,
        CODE_KEYBOARD_F1,
        CODE_KEYBOARD_F2,
        CODE_KEYBOARD_F3,
        CODE_KEYBOARD_F4,
        CODE_KEYBOARD_F5,
        CODE_KEYBOARD_F6,
        CODE_KEYBOARD_F7,
        CODE_KEYBOARD_F8,
        CODE_KEYBOARD_F9,
        CODE_KEYBOARD_F10,
        CODE_KEYBOARD_F11,
        CODE_KEYBOARD_F12,
        CODE_KEYBOARD_RIGHT,
        CODE_KEYBOARD_LEFT,
        CODE_KEYBOARD_DOWN,
        CODE_KEYBOARD_UP,
        CODE_KEYBOARD_RETURN = 13,
        CODE_KEYBOARD_ESCAPE = 27,
        CODE_KEYBOARD_BACKSPACE = 8,
        CODE_KEYBOARD_TAB = '\t',
        CODE_KEYBOARD_SPACE = ' ',
        CODE_KEYBOARD_GRAVE = '`',
        CODE_KEYBOARD_DELETE = 224,
        CODE_NO_ASCII = 1000,
        CODE_KEYBOARD_LSHIFT,
        CODE_KEYBOARD_RSHIFT,
        CODE_USER_DEFINED = 10000
    };

    enum Button {
        BUTTON_MOUSE_LEFT,
        BUTTON_MOUSE_MIDDLE,
        BUTTON_MOUSE_RIGHT
    };

    struct ButtonInfo {
        ButtonInfo(float x, float y, Button which);

        float _x, _y;
        Button _which;
    };

    struct MotionInfo {
        MotionInfo(float x, float y, Button which, uint32_t states);

        float _x, _y;
        Button _which;
        uint32_t _states;
    };

    union EventInfo {
        EventInfo(Code code);
        EventInfo(const ButtonInfo& button);
        EventInfo(const MotionInfo& motion);

        Code _code;
        ButtonInfo _button;
        MotionInfo _motion;
    };

    Event(Action action, uint32_t timestamp, const EventInfo& info);
    Event(const Event& other, float x, float y);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Event);

    bool ptin(const Rect& rectf) const;

//  [[script::bindings::property]]
    Event::Action action() const;
//  [[script::bindings::property]]
    float x() const;
//  [[script::bindings::property]]
    float y() const;
//  [[script::bindings::property]]
    V2 xy() const;

//  [[script::bindings::property]]
    uint32_t timestamp() const;

//  [[script::bindings::property]]
    Event::Code code() const;
//  [[script::bindings::property]]
    Event::Button button() const;

    wchar_t toCharacter() const;

private:
    Action _action;
    uint32_t _timestamp;
    EventInfo _info;
};

}

#endif
