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

#include "python/api.h"
#include "python/forwarding.h"
#include "python/extension/wrapper.h"

namespace ark {
namespace plugin {
namespace python {

class ARK_PLUGIN_PYTHON_API PyArkType {
public:
    typedef struct {
        PyObject_HEAD
        Box* box;
        Wrapper* wrapper;
        PyObject* weakreflist;

        template<typename T> const sp<T>& unpack() const {
            return box->unpack<T>();
        }

        template<typename T> sp<T>& unpack() {
            return box->unpack<T>();
        }

        template<typename T> void pack(const sp<T>& ptr) {
            *box = ptr.pack();
        }

        Wrapper* getWrapper() {
            if(wrapper)
                return wrapper;
            if(PyObject_IS_GC(reinterpret_cast<PyObject*>(this)))
                wrapper = new Wrapper();
            return wrapper;
        }

        void setContainerObject(const Box& object) {
            DCHECK(object.typeId() == Type<PyInstance>::id(), "Object is not a PyInstance");
            Wrapper* gcContainer = getWrapper();
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
    std::map<String, uint32_t> _constants;
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
