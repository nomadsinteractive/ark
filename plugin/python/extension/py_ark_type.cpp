#include "python/extension/py_ark_type.h"

#include "core/base/scope.h"

#include "python/extension/python_interpreter.h"
#include "python/extension/py_garbage_collector.h"

namespace ark {
namespace plugin {
namespace python {

static Py_hash_t __hash__(PyArkType::Instance* self)
{
    return reinterpret_cast<Py_hash_t>(self->box->ptr());
}

static int __traverse__(PyArkType::Instance* self, visitproc visitor, void* args)
{
    return self->container ? self->container->traverse(visitor, args) : 0;
}

static int __clear__(PyArkType::Instance* self)
{
    return self->container ? self->container->clear() : 0;
}

PyArkType::PyArkType(const String& name, const String& doc, unsigned long flags)
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
    pyType.tp_base = basetype();

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

PyObject* PyArkType::load(Instance& inst, const String& loader, TypeId typeId, const String& id, const sp<Scope>& args) const
{
    const std::map<TypeId, LoaderFunction>& functions = getLoader(loader);
    const auto iter = functions.find(typeId);
    DCHECK(iter != functions.end(), "Loader \"%s\" has no LoaderFunction for %d", loader.c_str(), typeId);
    return wrap(inst, iter->second(inst, id, args), args);
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

PyObject* PyArkType::wrap(Instance& inst, const Box& box, const sp<Scope>& args) const
{
    PyObject* bean = PythonInterpreter::instance()->toPyObject(box);
    if(args)
    {
        Instance* beanInst = reinterpret_cast<Instance*>(bean);
        PyContainer* beanWrapper = beanInst->getContainer();
        PyContainer* wrapper = beanWrapper ? beanWrapper : inst.getContainer();
        NOT_NULL(wrapper);
        for(auto iter = args->variables().begin(); iter != args->variables().end(); ++iter)
        {
            const sp<PyGarbageCollector> collector = iter->second.as<PyGarbageCollector>();
            if(collector)
                wrapper->addCollector(collector);
        }
    }
    return bean;
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
        nullptr,                                            /* tp_print */
        nullptr,                                            /* tp_getattr */
        nullptr,                                            /* tp_setattr */
        nullptr,                                            /* tp_reserved */
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
        nullptr,                                            /* tp_richcompare */
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
    self->container = nullptr;
    self->weakreflist = nullptr;
    return obj;
}

void PyArkType::__dealloc__(Instance* self)
{
    delete self->box;
    delete self->container;

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
        if(PythonInterpreter::instance()->isInstance<Lifecycle>(arg1)) {
            typename PyArkType::Instance* instance = reinterpret_cast<PyArkType::Instance*>(arg1);
            self->box->interfaces()->absorb(*instance->box);
        }
    }
    Py_INCREF(self);
    return reinterpret_cast<PyObject*>(self);
}

PyObject* PyArkType::__dispose__(PyArkType::Instance* self, PyObject* args, PyObject* kwargs)
{
    Py_RETURN_NONE;
}

}
}
}
