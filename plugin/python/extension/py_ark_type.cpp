#include "python/extension/py_ark_type.h"

#include "core/base/scope.h"
#include "core/components/discarded.h"
#include "core/inf/debris.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "python/extension/py_cast.h"
#include "python/extension/py_instance_ref.h"

namespace ark::plugin::python {

namespace {

Py_hash_t __hash__(const PyArkType::Instance* self)
{
    return self->box ? static_cast<Py_hash_t>(std::hash<uintptr_t>()(self->box->id())) : 0;
}

PyObject* __richcmp__(PyArkType::Instance* obj1, PyObject* obj2, const int32_t op)
{
    if(PyIndex_Check(reinterpret_cast<PyObject*>(obj1)) && PyIndex_Check(obj2))
    {
        PyObject* idx1 = PyNumber_Index(reinterpret_cast<PyObject*>(obj1));
        PyObject* idx2 = PyNumber_Index(obj2);
        const long value1 = PyLong_AsLong(idx1);
        const long value2 = PyLong_AsLong(idx2);
        Py_XDECREF(idx1);
        Py_XDECREF(idx2);
        Py_RETURN_RICHCOMPARE(value1, value2, op);
    }

    const bool isObj2ArkType = PythonExtension::instance().isPyArkTypeObject(Py_TYPE(obj2));
    const Py_hash_t hash1 = __hash__(obj1);
    const Py_hash_t hash2 = isObj2ArkType ? __hash__(reinterpret_cast<PyArkType::Instance*>(obj2)) : PyObject_Hash(obj2);

    Py_RETURN_RICHCOMPARE(hash1, hash2, op);
}

struct BreakException {
    int32_t _retcode;
};

int __traverse__(const PyArkType::Instance* self, visitproc visitor, void* args)
{
    if(const sp<Debris> debris = self->box->as<Debris>())
        try {
            debris->traverse([&visitor, args](const Box& packed) {
                if(const sp<PyInstanceRef> pi = packed.as<PyInstanceRef>())
                {
                    if(const int32_t vret = visitor(pi->instance(), args))
                        throw BreakException{vret};
                }
            });
        } catch(const BreakException& e) {
            return e._retcode;
        }
    return 0;
}

int __clear__(const PyArkType::Instance* self)
{
    if(const sp<Debris> debris = self->box->as<Debris>())
        debris->traverse([](const Box& packed) {
                if(const sp<PyInstanceRef> pi = packed.as<PyInstanceRef>())
                {
                    pi->clear();
                    return true;
                }
                return false;
            });
    return 0;
}

struct TypeLoader {
    Map<TypeId, PyArkType::LoaderFunction> _functions;
};

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

Map<TypeId, PyArkType::LoaderFunction>& PyArkType::loaders()
{
    return Global<TypeLoader>()->_functions;
}

void PyArkType::onReady()
{
    PyArkType* enumType = PythonExtension::instance().getPyArkType<Enum>();
    for(const auto& [name, value] : _enum_constants)
        PyDict_SetItemString(_py_type_object.tp_dict, name.c_str(), enumType->create(value));
}

int32_t PyArkType::doReady(TypeId typeId)
{
    _type_id = typeId;
    return PyType_Ready(&_py_type_object);
}

PyObject* PyArkType::load(Instance& inst, const TypeId typeId, const String& id, const Scope& args) const
{
    const Map<TypeId, LoaderFunction>& functions = loaders();
    const auto iter = functions.find(typeId);
    CHECK(iter != functions.end(), "No LoaderFunction defined for %d", typeId);
    return PythonExtension::instance().toPyObject(iter->second(inst, id, args));
}

PyTypeObject* PyArkType::basetype()
{
    static PyMethodDef PyArkType_methods[] = {
        {nullptr, nullptr, 0, nullptr}
    };

    static PyTypeObject ark_basetype = {
        PyVarObject_HEAD_INIT(nullptr, 0)
        "ark.Type",
        sizeof(Instance),
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

}
