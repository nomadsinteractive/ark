#ifndef ARK_PLUGIN_PYTHON_EXTENSION_REFERENCE_MANAGER_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_REFERENCE_MANAGER_H_

#include <vector>

#include "core/forwarding.h"
#include "core/types/weak_ptr.h"

namespace ark {
namespace plugin {
namespace python {

class PyInstance;

class ReferenceManager {
public:
//  [[script::bindings::property]]
    std::vector<sp<PyInstance>> refs();

    void track(const WeakPtr<PyInstance>& ref);

private:
    std::vector<WeakPtr<PyInstance>> _refs;
};

}
}
}

#endif
