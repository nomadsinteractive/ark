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
    virtual ~PyGarbageCollector() = default;

    virtual int traverse(visitproc visit, void* arg) = 0;
    virtual int clear() = 0;
};

class PyGarbageCollectorImpl : public PyGarbageCollector {
public:
    PyGarbageCollectorImpl(const sp<PyInstance>& garbage);

    virtual int traverse(visitproc visit, void* arg) override;
    virtual int clear() override;

private:
    sp<PyInstance> _garbage;

};

}
}
}

#endif
