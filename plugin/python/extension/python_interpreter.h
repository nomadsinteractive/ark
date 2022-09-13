#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PYTHON_INTERPRETER_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PYTHON_INTERPRETER_H_

#include <map>
#include <type_traits>

#include <Python.h>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"

#include "app/forwarding.h"

#include "python/api.h"
#include "python/forwarding.h"
#include "python/extension/py_ark_type.h"

namespace ark {
namespace plugin {
namespace python {

class ARK_PLUGIN_PYTHON_API PythonInterpreter {
public:

    static const sp<PythonInterpreter>& instance();

    PythonInterpreter();

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
        TypeId typeId = object.ensureInterfaces()->typeId();
        return _type_by_id.find(typeId) != _type_by_id.end() ? toPyObject(object.ensureInterfaces()->as(object, typeId)) : getPyArkType<T>()->create(object);
    }

    template<typename T, typename P> void pyModuleAddType(PyObject* module, const char* moduleName, const char* typeName, PyTypeObject* base, long flags) {
        static T pyType(Strings::sprintf("%s.%s", moduleName, typeName), Strings::sprintf("%s.%s Type", moduleName, typeName), base, flags);
        int ret = addPyArkType<P>(&pyType);
        DCHECK(!ret, "PyArkType init failed");
        if(!ret)
            PyModule_AddObject(module, typeName, pyType.getPyObject());
    }

    bool isPyArkTypeObject(void* pyTypeObject) const;
    PyArkType* getPyArkType(PyObject* pyObject);
    PyArkType* ensurePyArkType(PyObject* pyObject);

    PyObject* toPyObject(const Box& box);
    bool isPyObject(TypeId type) const;

    const sp<ReferenceManager>& referenceManager() const;

    void logErr() const;
    bool exceptErr(PyObject* type) const;

private:
    std::map<TypeId, PyArkType*> _type_by_id;
    std::map<void*, PyArkType*> _type_by_py_object;

    sp<ReferenceManager> _reference_manager;
};

}
}
}

#endif
