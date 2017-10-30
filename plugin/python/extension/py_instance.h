#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PY_INSTANCE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PY_INSTANCE_H_

#include <Python.h>

namespace ark {
namespace plugin {
namespace python {

class PyInstance {
public:
    struct Instance {
    public:
        Instance(PyObject* object, bool isBorrowedReference);
        Instance(const Instance& other);

        PyObject* _object;
        bool _is_borrowed_reference;
    };

public:
    PyInstance(const PyInstance& other);
    PyInstance(const Instance& instance);
    ~PyInstance();

    static Instance borrow(PyObject* object);
    static Instance steal(PyObject* object);
    static Instance own(PyObject* object);

    operator PyObject*();
    operator bool();

    PyObject* instance();
    PyObject* type();
    const char* name();

    bool hasAttr(const char* name) const;
    PyObject* getAttr(const char* name) const;
    PyObject* getAttr(PyObject* name) const;
    void setAttr(const char* name, PyObject* attr);
    int setAttr(PyObject* name, PyObject* attr);

    PyObject* call(PyObject* args);

    bool isCallable();
    long hash();

    PyObject* release();

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
