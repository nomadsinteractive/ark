#pragma once

#include <Python.h>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "python/forwarding.h"
#include "python/extension/py_instance_ref.h"

namespace ark::plugin::python {

class ARK_PLUGIN_PYTHON_API PyInstance {
public:
    PyInstance() = default;
    PyInstance(sp<PyInstanceRef> ref);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(PyInstance);

    static PyInstance borrow(PyObject* object);
    static PyInstance steal(PyObject* object);
    static PyInstance own(PyObject* object);

    static PyInstance track(PyObject* object);

    const sp<PyInstanceRef>& ref() const;

    Box toBox() const;

    PyObject* pyObject() const;
    explicit operator bool() const;

    PyObject* type();
    const char* name();

    bool hasAttr(const char* name) const;
    PyInstance getAttr(const char* name) const;

    PyObject* call(PyObject* args) const;

    bool isCallable() const;
    bool isList() const;

    bool isNone() const;
    bool isNullptr() const;

private:
    sp<PyInstanceRef> _ref;
};

}
