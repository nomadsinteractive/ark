#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API WithCallback {
public:
    WithCallback(sp<Runnable> callback);

    void notify() const;

private:
    sp<Runnable> _callback;
};

}
