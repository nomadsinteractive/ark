#include "python/impl/script/python_script.h"

#include "core/base/plugin_manager.h"
#include "core/base/scope.h"
#include "core/base/thread.h"
#include "core/types/null.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/forwarding.h"

#include "python/extension/arkmodule.h"
#include "python/extension/python_interpreter.h"
#include "python/extension/ark_py_importlib.h"
#include "python/extension/py_instance.h"

#include "platform/platform.h"


#ifdef __cplusplus
extern "C" {
#endif
extern PyObject* PyInit_ark();
#ifdef __cplusplus
}
#endif

namespace ark {
namespace plugin {
namespace python {

static struct _frozen _injected_frozen[10];

PythonScript::PythonScript(const String& name, const document& libraries)
    : _name(Strings::fromUTF8(name))
{
    DSET_THREAD_FLAG();
    PyImport_AppendInittab("ark", PyInit_ark);
    memset(_injected_frozen, 0, sizeof(_injected_frozen));
    for(struct _frozen* pt = (struct _frozen *) PyImport_FrozenModules, *i = _injected_frozen; pt->name; pt ++, i ++)
    {
        i->code = pt->code;
        i->size = pt->size;
        i->name = pt->name;
        if(strcmp(pt->name, "_frozen_importlib") == 0)
        {
            i->code = _Py_M__importlib;
            i->size = sizeof(_Py_M__importlib);
            i++;
            i->code = pt->code;
            i->size = pt->size;
            i->name = "_frozen_importlib_org";
        }
    }
    List<String> paths;
    for(const document& i : libraries->children("library"))
    {
        const String& v = Documents::ensureAttribute(i, "path");
        paths.push_back(v);
    }
    setPythonPath(paths);
    PyImport_FrozenModules = _injected_frozen;
    if(!Py_HasFileSystemDefaultEncoding)
    {
        char* encodings = reinterpret_cast<char*>(PyMem_RawMalloc(8));
        strncpy(encodings, "utf-8", 8);
        Py_FileSystemDefaultEncoding = encodings;
    }
    Py_SetProgramName((wchar_t*) (_name.c_str()));
    Py_NoSiteFlag = 1;
    Py_InitializeEx(0);
    _ark_module = PyImport_ImportModule("ark");
    Py_XINCREF(_ark_module);
}

PythonScript::~PythonScript()
{
    Py_XDECREF(_ark_module);
    Py_Finalize();
}

void PythonScript::addScopeToDict(PyObject* dict, const sp<Scope>& scope)
{
    for(auto iter : scope->variables())
    {
        const String& name = iter.first;
        const Box& box = iter.second;
        PyObject* pyInstance = PythonInterpreter::instance()->toPyObject(box);
        PyDict_SetItemString(dict, name.c_str(), pyInstance);
    }
}

PyObject* PythonScript::argumentsToTuple(const Script::Arguments& args)
{
    PyObject* tuple = PyTuple_New(args.size());
    uint32_t i = 0;
    for(const Box& arg : args)
        PyTuple_SetItem(tuple, i++, PythonInterpreter::instance()->toPyObject(arg));
    return tuple;
}

void PythonScript::run(const String& script, const sp<Scope>& vars)
{
    DCHECK_THREAD_FLAG();
    PyObject* m = PyImport_AddModule("__main__");
    DCHECK(m, "Module '__main__' does not exist");

    PyObject* globals = PyModule_GetDict(m);
    addScopeToDict(globals, vars);
    PyObject* v = PyRun_StringFlags(script.c_str(), Py_file_input, globals, globals, nullptr);
    if (v == NULL)
    {
        PythonInterpreter::instance()->logErr();
        return;
    }
    Py_DECREF(v);
}

Box PythonScript::call(const String& function, const Script::Arguments& args)
{
    DCHECK_THREAD_FLAG();
    PyObject* m = PyImport_AddModule("__main__");
    DCHECK(m, "Module '__main__' does not exist");

    PyObject* globals = PyModule_GetDict(m);
    PyInstance tuple = PyInstance::steal(argumentsToTuple(args));
    PyInstance func = PyInstance::borrow(PyDict_GetItemString(globals, function.c_str()));
    Box r;
    if(func && func.isCallable())
    {
        PyObject* ret = func.call(tuple);
        if(ret)
        {
            PyObject* type = reinterpret_cast<PyObject*>(ret->ob_type);
            if(PythonInterpreter::instance()->isPyArkTypeObject(type))
            {
                PyArkType::Instance* inst = reinterpret_cast<PyArkType::Instance*>(ret);
                r = Box(*inst->box);
            }
            else if(ret != Py_None && !PyBool_Check(ret))
                DFATAL("Unknow PyObject");
            Py_DECREF(ret);
        }
    }
    if(PyErr_Occurred())
        PythonInterpreter::instance()->logErr();
    return r;
}

PyObject* PythonScript::arkModule()
{
    return _ark_module;
}

PythonScript::BUILDER::BUILDER(BeanFactory& /*parent*/, const document& doc)
    : _manifest(doc)
{
}

sp<Script> PythonScript::BUILDER::build(const sp<Scope>& args)
{
    const sp<Script> script = sp<PythonScript>::make("ark-python", _manifest);
    const Global<PluginManager> pluginManager;
    pluginManager->each([script](const sp<Plugin>& plugin) {
        plugin->createScriptModule(script);
        return true;
    });
    return script;
}

}
}
}
