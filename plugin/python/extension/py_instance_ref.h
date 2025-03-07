#pragma once

#include <Python.h>

#include "core/forwarding.h"

#include "python/api.h"

namespace ark::plugin::python {

class ARK_PLUGIN_PYTHON_API PyInstanceRef {
public:
    PyInstanceRef(PyObject* instance, bool ownership);
    ~PyInstanceRef();
    DISALLOW_COPY_AND_ASSIGN(PyInstanceRef);

    PyObject* instance() const;
    void clear();

    bool hasOwnership() const;

private:
    PyObject* _instance;
    bool _ownership;
};

}
