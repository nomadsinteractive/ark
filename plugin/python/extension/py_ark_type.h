#ifndef ARK_PLUGIN_PYTHON_EXTENSION_EXTENSION_UTIL_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_EXTENSION_UTIL_H_

#include <map>
#include <functional>

#include <Python.h>

#include "core/forwarding.h"
#include "core/collection/by_type.h"
#include "core/base/string.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "python/api.h"
#include "python/forwarding.h"
#include "python/extension/py_container.h"

namespace ark {
namespace plugin {
namespace python {

class PyGarbageCollector;

class ARK_PLUGIN_PYTHON_API PyArkType {
public:
    typedef struct {
        PyObject_HEAD
        Box* box;
        PyContainer* container;
        PyObject* weakreflist;

        template<typename T> const sp<T>& unpack() const {
            DCHECK(typeCheck<T>(), "PyObject \"%s\" cannot being casted to %s", ob_base.ob_type->tp_name, Class::getClass<T>()->name());
            return box->unpack<T>();
        }

        template<typename T> sp<T>& unpack() {
            DCHECK(typeCheck<T>(), "PyObject \"%s\" cannot being casted to %s", ob_base.ob_type->tp_name, Class::getClass<T>()->name());
            return box->unpack<T>();
        }

        template<typename T> void pack(const sp<T>& ptr) {
            NOT_NULL(box);
            *box = ptr.pack();
        }

        template<typename T> bool typeCheck() const {
            return box && box->typeId() == Type<T>::id();
        }

        PyContainer* getContainer() {
            if(container)
                return container;
            if(PyObject_IS_GC(reinterpret_cast<PyObject*>(this)))
                container = new PyContainer();
            return container;
        }

        template<typename T> void addObjectToContainer(const sp<T>& object) {
            const sp<PyGarbageCollector> gc = object.as<PyGarbageCollector>();
            if(gc) {
                PyContainer* gcContainer = getContainer();
                DCHECK(gcContainer, "Object doesnot support GC");
                gcContainer->addCollector(gc);
            }
        }

        void addObjectToContainer(float /*object*/) {
        }
        void addObjectToContainer(const V2& /*object*/) {
        }

        void addObjectToContainer(const Box& object) {
            DCHECK(object.typeId() == Type<PyInstance>::id(), "Object is not a PyInstance");
            PyContainer* gcContainer = getContainer();
            DCHECK(gcContainer, "Object doesnot support GC");
            gcContainer->setPyInstance(object.as<PyInstance>());
        }

    } Instance;

public:
    PyArkType(const String& name, const String& doc, long flags);

    template<typename T> int ready() {
        _type_id = Type<T>::id();

        int r = PyType_Ready(&_py_type_object);
        doInitConstants();
        return r;
    }

    TypeId typeId() const;

    PyObject* load(Instance& inst, const String& loader, TypeId typeId, const String& id, const sp<Scope>& args) const;

    PyObject* getPyObject();
    PyTypeObject* getPyTypeObject();

    PyObject* create(const Box& box);

    typedef Box (*LoaderFunction)(Instance&, const String&, const sp<Scope>&);
    std::map<TypeId, LoaderFunction>& ensureLoader(const String& name);

protected:
    std::map<String, int32_t> _constants;
    std::map<String, std::map<TypeId, LoaderFunction>> _loaders;

private:
    void doInitConstants();

    const std::map<TypeId, LoaderFunction>& getLoader(const String& name) const;

    static PyObject* _py_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
    static int _py_init(Instance* self, PyObject* args, PyObject* kwds);
    static void _py_dealloc(Instance* self);

    PyObject* wrap(Instance& inst, const Box& box, const sp<Scope>& args) const;

private:
    String _name;
    String _doc;
    TypeId _type_id;

    PyTypeObject _py_type_object;
};

}
}
}

#endif
