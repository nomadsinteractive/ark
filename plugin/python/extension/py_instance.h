#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PY_INSTANCE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PY_INSTANCE_H_

#include <Python.h>

#include "core/forwarding.h"

namespace ark {
namespace plugin {
namespace python {

class PyInstance {
public:
    PyInstance(const PyInstance& other) = delete;
    PyInstance(PyInstance&& other);
    ~PyInstance();

    static PyInstance borrow(PyObject* object);
    static PyInstance steal(PyObject* object);

    static sp<PyInstance> adopt(PyObject* object);

    operator PyObject*();
    explicit operator bool();

    PyObject* object();
    PyObject* type();
    const char* name();

    bool hasAttr(const char* name) const;
    sp<PyInstance> getAttr(const char* name) const;

    PyObject* call(PyObject* args);

    bool isCallable();

    PyObject* release();
    void deref();

private:
    PyInstance(PyObject* object, bool isBorrowedReference);

private:
    PyObject* _object;
    bool _is_borrowed_reference;
};

}
}
}

#endif
