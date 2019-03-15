#ifndef ARK_PLUGIN_PYTHON_EXTENSION_WRAPPER_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_WRAPPER_H_

#include <Python.h>

#include <list>

#include "core/types/weak_ptr.h"
#include "core/types/shared_ptr.h"

#include "python/api.h"

namespace ark {
namespace plugin {
namespace python {

class PyGarbageCollector;
class PyInstance;

class ARK_PLUGIN_PYTHON_API PyContainer {
public:

    int traverse(visitproc visit, void* args);
    int clear();

    void addCollector(const WeakPtr<PyGarbageCollector>& collector);
    void setTag(const sp<PyInstance>& pyInstance);

private:
    std::list<WeakPtr<PyGarbageCollector>> _garbage_collectors;
    sp<PyInstance> _tag;
};

}
}
}

#endif
