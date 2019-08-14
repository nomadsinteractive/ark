#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PY_INSTANCE_REF_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PY_INSTANCE_REF_H_

#include <Python.h>

#include "core/forwarding.h"

#include "python/api.h"

namespace ark {
namespace plugin {
namespace python {

class ARK_PLUGIN_PYTHON_API PyInstanceRef {
public:
    PyInstanceRef(PyObject* instance);
    virtual ~PyInstanceRef() = default;

    virtual void clear();

    PyObject* instance();

protected:
    PyObject* _instance;

};

}
}
}

#endif
