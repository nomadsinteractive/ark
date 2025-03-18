#pragma once

#include "core/forwarding.h"
#include "core/types/weak_ptr.h"

#include "python/forwarding.h"

namespace ark::plugin::python {

class ReferenceManager {
public:
//  [[script::bindings::property]]
    Vector<sp<PyInstanceRef>> refs();

    void track(const WeakPtr<PyInstanceRef>& ref);

private:
    Vector<WeakPtr<PyInstanceRef>> _refs;
};

}
