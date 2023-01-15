#include "python/extension/py_ark_type.h"

#include "core/base/scope.h"
#include "core/epi/disposed.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

#include "python/extension/py_cast.h"
#include "python/extension/py_instance_ref.h"

namespace ark {
namespace plugin {
namespace python {

static PyObject* __richcmp__(PyArkType::Instance* obj1, PyObject* obj2, int op)
{
    bool obj2IsNone = obj2 == Py_None;
    if(!(obj2IsNone || PythonInterpreter::instance()->isPyArkTypeObject(Py_TYPE(obj2))))
    {
        LOGW("Comparing Ark-Type object \"%s\" with non-Ark-Type object \"%s\"", Py_TYPE(obj1)->tp_name, Py_TYPE(obj2)->tp_name);
        Py_RETURN_NOTIMPLEMENTED;
    }

    Py_hash_t hash1 = reinterpret_cast<Py_hash_t>(obj1->box->ptr());
    Py_hash_t hash2 = obj2IsNone ? PyObject_Hash(obj2) : reinterpret_cast<Py_hash_t>(reinterpret_cast<PyArkType::Instance*>(obj2)->box->ptr());

    Py_RETURN_RICHCOMPARE(hash1, hash2, op);
}

static Py_hash_t __hash__(PyArkType::Instance* self)
{
    return reinterpret_cast<Py_hash_t>(self->box->ptr());
}

struct BreakException {
    BreakException(int32_t retcode)
        : _retcode(retcode) {
    }

    int32_t _retcode;
};

static int __traverse__(PyArkType::Instance* self, visitproc visitor, void* args)
{
    const sp<Holder> holder = self->box->as<Holder>();
    if(holder)
        try {
            holder->traverse([&visitor, args](const Box& packed) {
                const sp<PyInstanceRef> pi = packed.as<PyInstanceRef>();
                if(pi) {
                    int32_t vret = visitor(pi->instance(), args);
                    if(vret)
                        throw BreakException(vret);
                    return true;
                }
                return false;
            });
        } catch(const BreakException& e) {
            return e._retcode;
        }
    return 0;
}

static int __clear__(PyArkType::Instance* self)
{
    const sp<Holder> holder = self->box->as<Holder>();
    if(holder)
        holder->traverse([](const Box& packed) {
                const sp<PyInstanceRef> pi = packed.as<PyInstanceRef>();
                if(pi) {
                    pi->clear();
                    return true;
                }
                return false;
            });
    return 0;
}

PyArkType::PyArkType(const String& name, const String& doc, PyTypeObject* base, unsigned long flags)
    : _name(name), _doc(doc) {
    PyTypeObject pyType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        _name.c_str(),          /*tp_name*/
        sizeof(Instance)        /*tp_basicsize*/
    };
    memset(&pyType.tp_itemsize, 0, sizeof(PyTypeObject) - offsetof(PyTypeObject, tp_itemsize));
    pyType.tp_flags = flags;
    pyType.tp_doc = _doc.c_str();
    if((flags & Py_TPFLAGS_HAVE_GC) != 0)
    {
        pyType.tp_traverse = reinterpret_cast<traverseproc>(__traverse__);
        pyType.tp_clear = reinterpret_cast<inquiry>(__clear__);
    }
    pyType.tp_base = base ? base : basetype();

    _py_type_object = pyType;
    Py_INCREF(&_py_type_object);
}

TypeId PyArkType::typeId() const
{
    return _type_id;
}

PyObject* PyArkType::getPyObject()
{
    return reinterpret_cast<PyObject*>(&_py_type_object);
}

PyTypeObject* PyArkType::getPyTypeObject()
{
    return &_py_type_object;
}

PyObject* PyArkType::create(const Box& box)
{
    Instance* self = reinterpret_cast<Instance*>(_py_type_object.tp_new(&_py_type_object, nullptr, nullptr));
    self->box = new Box(box);
    return reinterpret_cast<PyObject*>(self);
}

std::map<TypeId, PyArkType::LoaderFunction>& PyArkType::ensureLoader(const String& name)
{
    auto iter = _loaders.find(name);
    if(iter != _loaders.end())
        return iter->second;
    _loaders[name] = std::map<TypeId, LoaderFunction>();
    return _loaders[name];
}

const std::map<TypeId, PyArkType::LoaderFunction>& PyArkType::getLoader(const String& name) const
{
    auto iter = _loaders.find(name);
    DCHECK(iter != _loaders.end(), "Loader \"%s\" not found.", name.c_str());
    return iter->second;
}

PyObject* PyArkType::load(Instance& inst, const String& loader, TypeId typeId, const String& id, const Scope& args) const
{
    const std::map<TypeId, LoaderFunction>& functions = getLoader(loader);
    const auto iter = functions.find(typeId);
    DCHECK(iter != functions.end(), "Loader \"%s\" has no LoaderFunction for %d", loader.c_str(), typeId);
    return PythonInterpreter::instance()->toPyObject(iter->second(inst, id, args));
}

void PyArkType::doInitConstants()
{
    for(auto iter = _constants.begin(); iter != _constants.end(); ++iter)
    {
        const String& name = iter->first;
        int32_t value = iter->second;
        PyDict_SetItemString(_py_type_object.tp_dict, name.c_str(), PyLong_FromLong(value));
    }
}

PyTypeObject* PyArkType::basetype()
{
    static PyMethodDef PyArkType_methods[] = {
        {"absorb", reinterpret_cast<PyCFunction>(__absorb__), METH_VARARGS, nullptr},
        {"dispose", reinterpret_cast<PyCFunction>(__dispose__), METH_VARARGS, nullptr},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyTypeObject ark_basetype = {
        PyVarObject_HEAD_INIT(nullptr, 0)
        "ark.Type",
        sizeof(PyArkType::Instance),
        0,
        reinterpret_cast<destructor>(__dealloc__),          /* tp_dealloc */
        0,                                                  /* tp_vectorcall_offset */
        nullptr,                                            /* tp_getattr */
        nullptr,                                            /* tp_setattr */
        nullptr,                                            /* tp_as_async */
        nullptr,                                            /* tp_repr */
        nullptr,                                            /* tp_as_number */
        nullptr,                                            /* tp_as_sequence */
        nullptr,                                            /* tp_as_mapping */
        reinterpret_cast<hashfunc>(__hash__),               /* tp_hash */
        nullptr,                                            /* tp_call */
        nullptr,                                            /* tp_str */
        nullptr,                                            /* tp_getattro */
        nullptr,                                            /* tp_setattro */
        nullptr,                                            /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,           /* tp_flags */
        "ark.Type",                                         /* tp_doc */
        nullptr,                                            /* tp_traverse */
        nullptr,                                            /* tp_clear */
        reinterpret_cast<richcmpfunc>(__richcmp__),         /* tp_richcompare */
        offsetof(Instance, weakreflist),                    /* tp_weaklistoffset */
        nullptr,                                            /* tp_iter */
        nullptr,                                            /* tp_iternext */
        PyArkType_methods,                                  /* tp_methods */
        nullptr,                                            /* tp_members */
        nullptr,                                            /* tp_getset */
        nullptr,                                            /* tp_base */
        nullptr,                                            /* tp_dict */
        nullptr,                                            /* tp_descr_get */
        nullptr,                                            /* tp_descr_set */
        0,                                                  /* tp_dictoffset */
        reinterpret_cast<initproc>(__init__),               /* tp_init */
        nullptr,                                            /* tp_alloc */
        reinterpret_cast<newfunc>(__new__)                  /* tp_new */
    };

    return &ark_basetype;
}

PyObject* PyArkType::__new__(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    PyObject* obj = PyType_GenericNew(type, args, kwds);
    Instance* self = reinterpret_cast<Instance*>(obj);
    self->weakreflist = nullptr;
    return obj;
}

void PyArkType::__dealloc__(Instance* self)
{
    delete self->box;

    if (self->weakreflist)
        PyObject_ClearWeakRefs(reinterpret_cast<PyObject*>(self));

    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

int PyArkType::__init__(Instance* /*self*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    return 0;
}

PyObject* PyArkType::__absorb__(PyArkType::Instance* self, PyObject* args, PyObject* /*kwargs*/)
{
    PyObject* arg1;
    if(PyArg_ParseTuple(args, "O", &arg1)) {
        if(PythonInterpreter::instance()->isInstance<Disposed>(arg1)) {
            typename PyArkType::Instance* instance = reinterpret_cast<PyArkType::Instance*>(arg1);
            self->box->interfaces()->absorb(*instance->box);
        }
    }
    Py_INCREF(self);
    return reinterpret_cast<PyObject*>(self);
}

PyObject* PyArkType::__dispose__(PyArkType::Instance* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    const sp<Disposed> disposed = PyCast::ensureSharedPtr<Disposed>(reinterpret_cast<PyObject*>(self));
    disposed->dispose();
    Py_RETURN_NONE;
}

}
}
}
