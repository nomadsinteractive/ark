#ifndef ARK_APP_BASE_APPLICATION_CONTROLLER_H_
#define ARK_APP_BASE_APPLICATION_CONTROLLER_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API ApplicationController {
public:
//  [[script::bindings::enumeration]]
    enum SystemCursorName {
        SYSTEM_CURSOR_ARROW,
        SYSTEM_CURSOR_IBEAM,
        SYSTEM_CURSOR_WAIT,
        SYSTEM_CURSOR_CROSSHAIR,
        SYSTEM_CURSOR_WAITARROW,
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
    virtual sp<Object> createCursor(const sp<Bitmap>& bitmap, uint32_t hotX, uint32_t hotY) = 0;
//  [[script::bindings::auto]]
    virtual sp<Object> createSystemCursor(ApplicationController::SystemCursorName name) = 0;

//  [[script::bindings::auto]]
    virtual void showCursor(const sp<Object>& cursor) = 0;
//  [[script::bindings::auto]]
    virtual void hideCursor() = 0;
//  [[script::bindings::auto]]
    virtual void exit() = 0;

};

}

#endif
