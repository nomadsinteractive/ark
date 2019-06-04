#include "python/extension/py_ark_type.h"

#include "core/base/scope.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

#include "python/extension/python_interpreter.h"
#include "python/extension/py_garbage_collector.h"

namespace ark {
namespace plugin {
namespace python {

static PyObject* __richcmp__(PyArkType::Instance* obj1, PyArkType::Instance* obj2, int op)
{
    if(!PythonInterpreter::instance()->isPyArkTypeObject(Py_TYPE(obj2)))
    {
        LOGW("Comparing \"%s\" with \"%s\" is not supported", Py_TYPE(obj1)->tp_name, Py_TYPE(obj2)->tp_name);
        Py_RETURN_NOTIMPLEMENTED;
    }

    Py_hash_t hash1 = reinterpret_cast<Py_hash_t>(obj1->box->ptr());
    Py_hash_t hash2 = reinterpret_cast<Py_hash_t>(obj2->box->ptr());

    Py_RETURN_RICHCOMPARE(hash1, hash2, op);
}

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

static PyObject* __int__(PyArkType::Instance* self)
{
    const sp<Integer> obj = self->box->as<Integer>();
    DWARN(obj, "Unable to cast \"%s\" to Integer type", Py_TYPE(self)->tp_name);
    return PythonInterpreter::instance()->toPyObject<int32_t>(obj ? obj->val() : 0);
}

static PyObject* __float__(PyArkType::Instance* self)
{
    const sp<Numeric> obj = PythonInterpreter::instance()->toNumeric(reinterpret_cast<PyObject*>(self));
    DWARN(obj, "Unable to cast \"%s\" to Numeric type", Py_TYPE(self)->tp_name);
    return PythonInterpreter::instance()->toPyObject<float>(obj ? obj->val() : 0);
}

static PyNumberMethods PyArkType_tp_as_number = {
    nullptr,                                       /* binaryfunc nb_add;                  */ /* __add__ */
    nullptr,                                       /* binaryfunc nb_subtract;             */ /* __sub__ */
    nullptr,                                       /* binaryfunc nb_multiply;             */ /* __mul__ */
    nullptr,                                       /* binaryfunc nb_remainder;            */ /* __mod__ */
    nullptr,                                       /* binaryfunc nb_divmod;               */ /* __divmod__ */
    nullptr,                                       /* ternaryfunc nb_power;               */ /* __pow__ */
    nullptr,                                       /* unaryfunc nb_negative;              */ /* __neg__ */
    nullptr,                                       /* unaryfunc nb_positive;              */ /* __pos__ */
    nullptr,                                       /* unaryfunc nb_absolute;              */ /* __abs__ */
    nullptr,                                       /* inquiry nb_bool;                    */ /* __bool__ */
    nullptr,                                       /* unaryfunc nb_invert;                */ /* __invert__ */
    nullptr,                                       /* binaryfunc nb_lshift;               */ /* __lshift__ */
    nullptr,                                       /* binaryfunc nb_rshift;               */ /* __rshift__ */
    nullptr,                                       /* binaryfunc nb_and;                  */ /* __and__ */
    nullptr,                                       /* binaryfunc nb_xor;                  */ /* __xor__ */
    nullptr,                                       /* binaryfunc nb_or;                   */ /* __or__ */
    (unaryfunc) __int__,                           /* unaryfunc nb_int;                   */ /* __int__ */
    nullptr,                                       /* void *nb_reserved;                  */
    (unaryfunc) __float__,                         /* unaryfunc nb_float;                 */ /* __float__ */
    nullptr,                                       /* binaryfunc nb_inplace_add           */
    nullptr,                                       /* binaryfunc nb_inplace_subtract      */
    nullptr,                                       /* binaryfunc nb_inplace_multiply      */
    nullptr,                                       /* binaryfunc nb_inplace_remainder;    */
    nullptr,                                       /* ternaryfunc nb_inplace_power;       */
    nullptr,                                       /* binaryfunc nb_inplace_lshift;       */
    nullptr,                                       /* binaryfunc nb_inplace_rshift;       */
    nullptr,                                       /* binaryfunc nb_inplace_and;          */
    nullptr,                                       /* binaryfunc nb_inplace_xor;          */
    nullptr,                                       /* binaryfunc nb_inplace_or;           */
    nullptr,                                       /* binaryfunc nb_floor_divide;         */
    nullptr,                                       /* binaryfunc nb_true_divide;          */ /* __div__ */
    nullptr,                                       /* binaryfunc nb_inplace_floor_divide; */
    nullptr                                        /* binaryfunc nb_inplace_true_divide   */ /* __idiv__ */
};

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
    pyType.tp_as_number = &PyArkType_tp_as_number;
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
        DASSERT(wrapper);
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
        if(PythonInterpreter::instance()->isInstance<Disposed>(arg1)) {
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
