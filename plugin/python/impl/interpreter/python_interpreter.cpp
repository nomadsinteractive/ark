#include "python/impl/interpreter/python_interpreter.h"

#include "core/base/plugin_manager.h"
#include "core/base/thread.h"
#include "core/inf/asset.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "python/extension/python_extension.h"
#include "python/extension/py_cast.h"
#include "python/extension/py_instance.h"

#include "python/extension/py__ark_bootstrap.h"
#include "python/libs/runtime/py__ark_audit.h"

#ifdef __cplusplus
extern "C" {
#endif

PyMODINIT_FUNC PyInit_ark();
PyMODINIT_FUNC PyInit__ctypes(void);
PyMODINIT_FUNC PyInit__decimal(void);
PyMODINIT_FUNC PyInit__socket(void);
PyMODINIT_FUNC PyInit_select(void);
PyMODINIT_FUNC PyInit_pyexpat(void);
#ifdef WIN32
PyMODINIT_FUNC PyInit__overlapped(void);
#endif

PyAPI_DATA(const struct _frozen *) _PyImport_FrozenBootstrap;
PyAPI_DATA(const struct _frozen *) PyImport_FrozenModules;

#ifdef __cplusplus
}
#endif

namespace ark::plugin::python {

namespace {

_frozen _import_frozen_bootstrap[10];
_frozen _import_frozen_modules[16];

bool preInitialize()
{
    for(const _frozen* pt = _PyImport_FrozenBootstrap, *i = _import_frozen_bootstrap; pt->name; pt ++, i ++)
        if(strcmp(pt->name, "_frozen_importlib_org") == 0)
            return true;

    PyImport_AppendInittab("ark", PyInit_ark);
    PyImport_AppendInittab("_ctypes", PyInit__ctypes);
    PyImport_AppendInittab("_decimal", PyInit__decimal);
    PyImport_AppendInittab("_socket", PyInit__socket);
    PyImport_AppendInittab("select", PyInit_select);
    PyImport_AppendInittab("pyexpat", PyInit_pyexpat);
#ifdef WIN32
    PyImport_AppendInittab("_overlapped", PyInit__overlapped);
#endif

    memset(_import_frozen_modules, 0, sizeof(_import_frozen_modules));
    size_t i = 0;
    if(PyImport_FrozenModules)
        while(PyImport_FrozenModules[i].name)
        {
            _import_frozen_modules[i] = PyImport_FrozenModules[i];
            ++i;
        }

    if(i < sizeof(_import_frozen_modules) / sizeof(_frozen))
    {
        _import_frozen_modules[i] = {"_ark_audit", _Py_M___ark_audit, sizeof(_Py_M___ark_audit), 0, nullptr};
        PyImport_FrozenModules = _import_frozen_modules;
    }
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

PythonInterpreter::PythonInterpreter(const StringView name, const document& libraries)
    : _name(Strings::fromUTF8(name)), _ark_module(nullptr), _ark_audit_module(nullptr)
{
    if(!preInitialize())
    {
        memset(_import_frozen_bootstrap, 0, sizeof(_import_frozen_bootstrap));
        for(_frozen* pt = const_cast<_frozen*>(_PyImport_FrozenBootstrap), *i = _import_frozen_bootstrap; pt->name; pt ++, i ++)
        {
            *i = *pt;
            if(strcmp(pt->name, "_frozen_importlib") == 0)
            {
                i->code = _Py_M___ark_bootstrap;
                i->size = sizeof(_Py_M___ark_bootstrap);
                i->get_code = nullptr;
                i++;
                *i = *pt;
                i->name = "_frozen_importlib_org";
            }
        }
        _PyImport_FrozenBootstrap = _import_frozen_bootstrap;
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
    Py_XDECREF(_ark_audit_module);
    Py_Finalize();
}

void PythonInterpreter::initialize()
{
    DSET_THREAD_FLAG();

    Py_InitializeEx(0);
    _ark_module = PyImport_ImportModule("ark");
    Py_XINCREF(_ark_module);

#ifdef ARK_FLAG_PUBLISHING_BUILD
    _ark_audit_module = PyImport_ImportModule("_ark_audit");
    Py_XINCREF(_ark_audit_module);
#endif

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

PythonInterpreter::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& manifest)
    : _manifest(manifest)
{
}

sp<Interpreter> PythonInterpreter::BUILDER::build(const Scope& args)
{
    return sp<Interpreter>::make<PythonInterpreter>("ark-python", _manifest);
}

}
