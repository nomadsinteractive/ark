#include "python/impl/interpreter/python_interpreter.h"

#include "core/base/plugin_manager.h"
#include "core/base/scope.h"
#include "core/base/thread.h"
#include "core/inf/asset.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "python/extension/python_extension.h"
#include "python/extension/ark_py_importlib.h"
#include "python/extension/py_cast.h"
#include "python/extension/py_instance.h"


#ifdef __cplusplus
extern "C" {
#endif

PyMODINIT_FUNC PyInit_ark();
#ifndef ARK_FLAG_PUBLISHING_BUILD
PyMODINIT_FUNC PyInit__ctypes(void);
PyMODINIT_FUNC PyInit__decimal(void);
PyMODINIT_FUNC PyInit__socket(void);
PyMODINIT_FUNC PyInit_select(void);
PyMODINIT_FUNC PyInit_pyexpat(void);
#ifdef WIN32
PyMODINIT_FUNC PyInit__overlapped(void);
#endif
#endif

PyAPI_DATA(const struct _frozen *) _PyImport_FrozenBootstrap;
#ifdef __cplusplus
}
#endif

namespace ark::plugin::python {

namespace {

_frozen _injected_frozen[10];

bool hasInjected()
{
    for(const _frozen* pt = _PyImport_FrozenBootstrap, *i = _injected_frozen; pt->name; pt ++, i ++)
        if(strcmp(pt->name, "_frozen_importlib_org") == 0)
            return true;
    return false;
}

PyObject* argumentsToTuple(const Interpreter::Arguments& args)
{
    PyObject* tuple = PyTuple_New(args.size());
    uint32_t i = 0;
    for(const Box& arg : args)
        PyTuple_SetItem(tuple, i++, PythonExtension::instance().toPyObject(arg));
    return tuple;
}

PyInstance getMainModuleAttr(const char* name)
{
    PyObject* m = PyImport_AddModule("__main__");
    CHECK(m, "Module '__main__' does not exist");
    return PyInstance::own(PyDict_GetItemString(PyModule_GetDict(m), name));
}

}

PythonInterpreter::PythonInterpreter(StringView name, const document& libraries)
    : _name(Strings::fromUTF8(name))
{
    PyImport_AppendInittab("ark", PyInit_ark);
#ifndef ARK_FLAG_PUBLISHING_BUILD
    PyImport_AppendInittab("_ctypes", PyInit__ctypes);
    PyImport_AppendInittab("_decimal", PyInit__decimal);
    PyImport_AppendInittab("_socket", PyInit__socket);
    PyImport_AppendInittab("select", PyInit_select);
    PyImport_AppendInittab("pyexpat", PyInit_pyexpat);
#ifdef WIN32
   PyImport_AppendInittab("_overlapped", PyInit__overlapped);
#endif
#endif
    if(!hasInjected())
    {
        memset(_injected_frozen, 0, sizeof(_injected_frozen));
        for(_frozen* pt = const_cast<_frozen*>(_PyImport_FrozenBootstrap), *i = _injected_frozen; pt->name; pt ++, i ++)
        {
            *i = *pt;
            if(strcmp(pt->name, "_frozen_importlib") == 0)
            {
                i->code = _Py_M__ark_importlib;
                i->size = sizeof(_Py_M__ark_importlib);
                i->get_code = nullptr;
                i++;
                *i = *pt;
                i->name = "_frozen_importlib_org";
            }
        }
        _PyImport_FrozenBootstrap = _injected_frozen;
    }
    for(const document& i : libraries->children("library"))
    {
        const String& v = Documents::ensureAttribute(i, "path");
        _paths.push_back(v);
    }
    if(!Py_HasFileSystemDefaultEncoding)
    {
        char* encodings = static_cast<char*>(PyMem_RawMalloc(8));
        strncpy(encodings, "utf-8", 8);
        Py_FileSystemDefaultEncoding = encodings;
    }
#ifdef ARK_FLAG_PUBLISHING_BUILD
    Py_NoSiteFlag = 1;
#endif
}

PythonInterpreter::~PythonInterpreter()
{
    Py_XDECREF(_ark_module);
    Py_Finalize();
}

void PythonInterpreter::initialize()
{
    DSET_THREAD_FLAG();

    Py_InitializeEx(0);
    _ark_module = PyImport_ImportModule("ark");
    Py_XINCREF(_ark_module);

    const Global<PluginManager> pluginManager;
    for(Plugin& i : pluginManager->plugins())
        i.createScriptModule(*this);
}

void PythonInterpreter::execute(const sp<Asset>& source)
{
    DCHECK_THREAD_FLAG();
    PyObject* m = PyImport_AddModule("__main__");
    DCHECK(m, "Module '__main__' does not exist");

    PyObject* globals = PyModule_GetDict(m);
    LOGD("run script, location: %s", source->location().c_str());
    const PyInstance co = PyInstance::steal(Py_CompileStringExFlags(Strings::loadFromReadable(source->open()).c_str(), source->location().c_str(), Py_file_input, nullptr, -1));
    const PyInstance v = PyInstance::steal(PyEval_EvalCode(co.pyObject(), globals, globals));
    if(v.isNullptr() || v.pyObject() == nullptr)
        PythonExtension::instance().logErr();
}

Box PythonInterpreter::call(const Box& func, const Interpreter::Arguments& args)
{
    DCHECK_THREAD_FLAG();
    const PyInstance pyfunc = PyInstance::steal(PyCast::toPyObject(func));
    ASSERT(!pyfunc.isNullptr() && pyfunc.isCallable());

    const PyInstance tuple = PyInstance::steal(argumentsToTuple(args));
    if(PyObject* ret = pyfunc.call(tuple.pyObject()))
    {
        if(PyErr_Occurred())
            PythonExtension::instance().logErr();
        Box r = std::move(PyCast::toCppObject<Box>(ret).value());
        Py_DECREF(ret);
        return r;
    }
    PythonExtension::instance().logErr();
    return {};
}

Box PythonInterpreter::attr(const Box& obj, const StringView name)
{
    DCHECK_THREAD_FLAG();
    if(!obj)
        return getMainModuleAttr(name.data()).toBox();

    const PyInstance pyobj = PyInstance::steal(PyCast::toPyObject(obj));
    ASSERT(!pyobj.isNullptr());
    const PyInstance pyattr = pyobj.getAttr(name.data());
    if(PythonExtension::instance().exceptErr(PyExc_AttributeError))
        return {};
    return pyattr.toBox();
}

PyObject* PythonInterpreter::arkModule()
{
    return _ark_module;
}

const Vector<String>& PythonInterpreter::paths() const
{
    return _paths;
}

PythonInterpreter::BUILDER::BUILDER(BeanFactory& /*parent*/, const document& manifest)
    : _manifest(manifest)
{
}

sp<Interpreter> PythonInterpreter::BUILDER::build(const Scope& args)
{
    return sp<Interpreter>::make<PythonInterpreter>("ark-python", _manifest);
}

}
