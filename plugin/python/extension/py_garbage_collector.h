#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PY_GARBAGE_COLLECTABLE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PY_GARBAGE_COLLECTABLE_H_

#include <Python.h>

#include "core/types/shared_ptr.h"

#include "python/forwarding.h"

namespace ark {
namespace plugin {
namespace python {

class PyGarbageCollector {
public:
    PyGarbageCollector(const sp<PyInstance>& garbage);

    int traverse(visitproc visit, void* arg);
    int clear();

private:
    sp<PyInstance> _garbage;
};

}
}
}

#endif
