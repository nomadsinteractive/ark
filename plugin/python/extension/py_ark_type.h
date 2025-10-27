#pragma once

#include <Python.h>

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/collection/traits.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "python/api.h"
#include "python/forwarding.h"

namespace ark::plugin::python {

class ARK_PLUGIN_PYTHON_API PyArkType {
public:
    struct Instance {
        PyObject_HEAD
        Box* box;
        PyObject* weakreflist;

        template<typename T> sp<T> unpack() const {
            DCHECK(box, "PyObject \"%s\" has not been initialized", ob_base.ob_type->tp_name);
            CHECK(typeCheck<T>(), "PyObject \"%s\" cannot being casted to %s", ob_base.ob_type->tp_name, Class::ensureClass<T>()->name());
            return box->toPtr<T>();
        }

        template<typename T> sp<T> as() const {
            DCHECK(box, "PyObject \"%s\" has not been initialized", ob_base.ob_type->tp_name);
            const sp<T> inst = box->as<T>();
            CHECK(inst, "PyObject \"%s\" cannot being casted to %s", ob_base.ob_type->tp_name, Class::ensureClass<T>()->name());
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
        return doReady(Type<T>::id());
    }

    TypeId typeId() const;

    PyObject* load(Instance& inst, const String& loader, TypeId typeId, const String& id, const Scope& args) const;

    PyObject* getPyObject();
    PyTypeObject* getPyTypeObject();

    PyObject* create(const Box& box);

    typedef Box (*LoaderFunction)(Instance&, const String&, const Scope&);
    static Map<TypeId, LoaderFunction>& loaders();

    void onReady();

protected:
    Map<String, Box> _enum_constants;

private:
    int32_t doReady(TypeId typeId);

    static PyTypeObject* basetype();

    static PyObject* __new__(PyTypeObject *type, PyObject *args, PyObject *kwds);
    static int __init__(Instance* self, PyObject* args, PyObject* kwds);
    static void __dealloc__(Instance* self);

private:
    String _name;
    String _doc;
    TypeId _type_id;

    PyTypeObject _py_type_object;
};

}
