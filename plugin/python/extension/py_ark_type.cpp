#include "python/extension/py_ark_type.h"

#include "core/base/scope.h"

#include "python/extension/python_interpreter.h"
#include "python/extension/py_garbage_collector.h"

namespace ark {
namespace plugin {
namespace python {


static int __traverse__(PyArkType::Instance* self, visitproc visitor, void* args)
{
    return self->wrapper ? self->wrapper->traverse(visitor, args) : 0;
}

static int __clear__(PyArkType::Instance* self)
{
    return self->wrapper ? self->wrapper->clear() : 0;
}

PyArkType::PyArkType(const String& name, const String& doc, long flags)
    : _name(name), _doc(doc) {
    PyTypeObject pyType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        _name.c_str(),          /*tp_name*/
        sizeof(Instance)        /*tp_basicsize*/
    };
    memset(&pyType.tp_itemsize, 0, sizeof(PyTypeObject) - offsetof(PyTypeObject, tp_itemsize));
    pyType.tp_flags = flags;
    pyType.tp_doc = _doc.c_str();
    pyType.tp_new = (newfunc) _py_new;
    pyType.tp_init = (initproc) _py_init;
    pyType.tp_dealloc = (destructor) _py_dealloc;
    pyType.tp_weaklistoffset = offsetof(Instance, weakreflist);
    if((flags & Py_TPFLAGS_HAVE_GC) != 0)
    {
        pyType.tp_traverse = (traverseproc) __traverse__;
        pyType.tp_clear = (inquiry) __clear__;
    }

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
    Instance* self = reinterpret_cast<Instance*>(_py_type_object.tp_new(&_py_type_object, 0, 0));
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

PyObject* PyArkType::_py_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    PyObject* obj = PyType_GenericNew(type, args, kwds);
    Instance* self = reinterpret_cast<Instance*>(obj);
    self->wrapper = nullptr;
    self->weakreflist = nullptr;
    return obj;
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
        uint32_t value = iter->second;
        PyDict_SetItemString(_py_type_object.tp_dict, name.c_str(), PyLong_FromLong(value));
    }
}

void PyArkType::_py_dealloc(Instance* self)
{
    delete self->box;
    delete self->wrapper;

    if (self->weakreflist)
        PyObject_ClearWeakRefs(reinterpret_cast<PyObject*>(self));

    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject* PyArkType::wrap(Instance& inst, const Box& box, const sp<Scope>& args) const
{
    PyObject* bean = PythonInterpreter::instance()->toPyObject(box);
    if(args)
    {
        Instance* beanInst = reinterpret_cast<Instance*>(bean);
        Wrapper* beanWrapper = beanInst->getWrapper();
        Wrapper* wrapper = beanWrapper ? beanWrapper : inst.getWrapper();
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

int PyArkType::_py_init(Instance* self, PyObject* args, PyObject* kwds)
{
    return 0;
}

}
}
}
