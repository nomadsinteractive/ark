#pragma once

#include <Python.h>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"

#include "app/forwarding.h"

#include "python/api.h"
#include "python/forwarding.h"
#include "python/impl/interpreter/python_interpreter.h"
#include "python/extension/py_ark_type.h"
#include "python/extension/py_bridge.h"

namespace ark::plugin::python {

class ARK_PLUGIN_PYTHON_API PythonExtension {
public:

    static PythonExtension& instance();

    PythonExtension();

    template<typename T> int addPyArkType(PyArkType* pyArkType) {
        _type_by_id[Type<T>::id()] = pyArkType;
        _type_by_py_object[pyArkType->getPyTypeObject()] = pyArkType;
        return pyArkType->ready<T>();
    }

    template<typename T> PyArkType* getPyArkType() const {
        auto iter = _type_by_id.find(Type<T>::id());
        DCHECK(iter != _type_by_id.end(), "PyTypeObject not found");
        return reinterpret_cast<PyArkType*>(iter->second);
    }

    template<typename T> bool isInstance(PyObject* object) const {
        return PyObject_IsInstance(object, getPyArkType<T>()->getPyObject()) != 0;
    }

    template<typename T> PyObject* pyNewObject(const sp<T>& object) {
        const Class* objClass = object.getClass();
        const TypeId typeId = objClass->id();
        const Box box(object);
        return _type_by_id.find(typeId) != _type_by_id.end() ? toPyObject(objClass->cast(box, typeId)) : getPyArkType<T>()->create(box);
    }

    template<typename T, typename P> T* pyModuleAddType(PyObject* module, const char* moduleName, const char* typeName, PyTypeObject* base, long flags) {
        static T pyType(Strings::sprintf("%s.%s", moduleName, typeName), Strings::sprintf("%s.%s Type", moduleName, typeName), base, flags);
        const int ret = addPyArkType<P>(&pyType);
        DCHECK(!ret, "PyArkType init failed");
        if(!ret)
            PyBridge::PyModule_AddObject(module, typeName, pyType.getPyObject());
        return &pyType;
    }

    bool isPyArkTypeObject(void* pyTypeObject) const;
    PyArkType* getPyArkType(PyObject* pyObject);
    PyArkType* ensurePyArkType(PyObject* pyObject);

    PyObject* toPyObject(const Box& box);
    bool isPyObject(TypeId type) const;

    const sp<ReferenceManager>& referenceManager() const;

    void printStack() const;
    void logErr() const;
    void flushErr() const;
    bool exceptErr(PyObject* type) const;

    template<typename T> void addModulePlugin(T& plugin, Interpreter& script, const char* name, const char* documentation, const PyMethodDef* methods) {
        PythonInterpreter* pythonScript = static_cast<PythonInterpreter*>(&script);
        ASSERT(pythonScript);
        static struct PyModuleDef cPluginModuleDef = {
            PyModuleDef_HEAD_INIT,
            name,                /* name of module */
            documentation,       /* module documentation, may be NULL */
            -1,                  /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
            const_cast<PyMethodDef*>(methods)
        };

        PyObject* pluginModule = PyBridge::PyModule_Create2(&cPluginModuleDef, PYTHON_ABI_VERSION);
        plugin.initialize(pluginModule);
        PyBridge::PyModule_AddObject(pythonScript->arkModule(), name, pluginModule);
    }

private:
    Map<TypeId, PyArkType*> _type_by_id;
    Map<void*, PyArkType*> _type_by_py_object;

    sp<ReferenceManager> _reference_manager;
};

}
