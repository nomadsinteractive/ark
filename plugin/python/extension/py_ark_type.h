#pragma once

#include <map>

#include <Python.h>

#include "core/forwarding.h"
#include "core/collection/by_type.h"
#include "core/base/string.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "python/api.h"
#include "python/forwarding.h"

namespace ark {
namespace plugin {
namespace python {

class PyGarbageCollector;

class ARK_PLUGIN_PYTHON_API PyArkType {
public:
    struct Instance {
        PyObject_HEAD
        Box* box;
        PyObject* weakreflist;

        template<typename T> const sp<T>& unpack() const {
            DCHECK(box, "PyObject \"%s\" has not been initialized", ob_base.ob_type->tp_name);
            CHECK(typeCheck<T>(), "PyObject \"%s\" cannot being casted to %s", ob_base.ob_type->tp_name, Class::getClass<T>()->name());
            return box->unpack<T>();
        }

        template<typename T> sp<T> as() const {
            DCHECK(box, "PyObject \"%s\" has not been initialized", ob_base.ob_type->tp_name);
            const sp<T> inst = box->as<T>();
            CHECK(inst, "PyObject \"%s\" cannot being casted to %s", ob_base.ob_type->tp_name, Class::getClass<T>()->name());
            return inst;
        }

    private:
        template<typename T> bool typeCheck() const {
            return box && box->typeId() == Type<T>::id();
        }

    };

public:
    PyArkType(const String& name, const String& doc, PyTypeObject* base, unsigned long flags);

    template<typename T> int ready() {
        _type_id = Type<T>::id();
        return doReady();
    }

    TypeId typeId() const;

    PyObject* load(Instance& inst, const String& loader, TypeId typeId, const String& id, const Scope& args) const;

    PyObject* getPyObject();
    PyTypeObject* getPyTypeObject();

    PyObject* create(const Box& box);

    typedef Box (*LoaderFunction)(Instance&, const String&, const Scope&);
    std::map<TypeId, LoaderFunction>& ensureLoader(const String& name);

protected:
    std::map<String, int32_t> _enum_constants;
    std::map<String, String> _string_constants;

    std::map<String, std::map<TypeId, LoaderFunction>> _loaders;

private:
    int32_t doReady();

    const std::map<TypeId, LoaderFunction>& getLoader(const String& name) const;

    static PyTypeObject* basetype();

    static PyObject* __new__(PyTypeObject *type, PyObject *args, PyObject *kwds);
    static int __init__(Instance* self, PyObject* args, PyObject* kwds);
    static void __dealloc__(Instance* self);

    static PyObject* __absorb__(Instance* self, PyObject* args, PyObject* kwargs);
    static PyObject* __dispose__(Instance* self, PyObject* args, PyObject* kwargs);

private:
    String _name;
    String _doc;
    TypeId _type_id;

    PyTypeObject _py_type_object;
};

}
}
}
