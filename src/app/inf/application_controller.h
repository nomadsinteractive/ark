#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API ApplicationController {
public:
//  [[script::bindings::enumeration]]
    enum SystemCursorName {
        SYSTEM_CURSOR_DEFAULT,
        SYSTEM_CURSOR_TEXT,
        SYSTEM_CURSOR_WAIT,
        SYSTEM_CURSOR_CROSSHAIR,
        SYSTEM_CURSOR_PROGRESS,
        SYSTEM_CURSOR_SIZENWSE,
        SYSTEM_CURSOR_SIZENESW,
        SYSTEM_CURSOR_SIZEWE,
        SYSTEM_CURSOR_SIZENS,
        SYSTEM_CURSOR_SIZEALL,
        SYSTEM_CURSOR_NO,
        SYSTEM_CURSOR_HAND,
    };

public:
    virtual ~ApplicationController() = default;

//  [[script::bindings::auto]]
    virtual Box createCursor(const sp<Bitmap>& bitmap, int32_t hotX, int32_t hotY) = 0;
//  [[script::bindings::auto]]
    virtual Box createSystemCursor(ApplicationController::SystemCursorName name) = 0;

//  [[script::bindings::auto]]
    virtual void showCursor(const Box& cursor = nullptr) = 0;
//  [[script::bindings::auto]]
    virtual void hideCursor() = 0;

//  [[script::bindings::auto]]
    virtual void setMouseCapture(bool enabled) = 0;
//  [[script::bindings::auto]]
    virtual void setTextInput(bool enabled) = 0;

//  [[script::bindings::auto]]
    virtual void exit() = 0;

};

}
