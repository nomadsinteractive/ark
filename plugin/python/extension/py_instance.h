#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PY_INSTANCE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PY_INSTANCE_H_

#include <Python.h>

#include "core/forwarding.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "python/forwarding.h"
#include "python/extension/py_instance_ref.h"

namespace ark {
namespace plugin {
namespace python {

class PyInstance {
public:
    PyInstance();
    PyInstance(const sp<PyInstanceRef>& ref);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(PyInstance);

    static PyInstance borrow(PyObject* object);
    static PyInstance steal(PyObject* object);
    static PyInstance own(PyObject* object);

    static PyInstance track(PyObject* object);

    const sp<PyInstanceRef>& ref() const;

    PyObject* pyObject() const;
    explicit operator bool();

    PyObject* type();
    const char* name();

    bool hasAttr(const char* name) const;
    PyInstance getAttr(const char* name) const;

    PyObject* call(PyObject* args);

    bool isCallable() const;

    void clear();

private:
    class Borrowed : public PyInstanceRef, Implements<Borrowed, PyInstanceRef> {
    public:
        Borrowed(PyObject* object);

        virtual void clear();

    };

    class Owned : public PyInstanceRef, Implements<Owned, PyInstanceRef> {
    public:
        Owned(PyObject* object);
        ~Owned() override;

    };

    class Stolen : public PyInstanceRef, Implements<Stolen, PyInstanceRef> {
    public:
        Stolen(PyObject* object);
        ~Stolen() override;

    };

private:
    sp<PyInstanceRef> _ref;
};

}
}
}

#endif
