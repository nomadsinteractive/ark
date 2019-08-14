#ifndef ARK_PLUGIN_PYTHON_EXTENSION_REFERENCE_MANAGER_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_REFERENCE_MANAGER_H_

#include <vector>

#include "core/forwarding.h"
#include "core/types/weak_ptr.h"

#include "python/forwarding.h"

namespace ark {
namespace plugin {
namespace python {

class ReferenceManager {
public:
//  [[script::bindings::property]]
    std::vector<sp<PyInstanceRef>> refs();

    void track(const WeakPtr<PyInstanceRef>& ref);

private:
    std::vector<WeakPtr<PyInstanceRef>> _refs;
};

}
}
}

#endif
