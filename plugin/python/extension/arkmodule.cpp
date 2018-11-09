#include "python/extension/arkmodule.h"

#include "core/forwarding.h"
#include "core/ark.h"
#include "core/inf/dictionary.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"
#include "core/util/log.h"
#include "core/util/strings.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

#include "python/extension/py_instance.h"
#include "python/extension/asset_resource.h"
#include "python/extension/python_interpreter.h"
#include "python/extension/py_ark_type.h"

#include "platform/platform.h"

#include "generated/py_ark_bindings.h"

using namespace ark;

namespace ark {
namespace plugin {
namespace python {

class PyContainer;

static PyObject* ark_log(Log::LogLevel level, PyObject* args);
static PyObject* ark_logd(PyObject* self, PyObject* args);
static PyObject* ark_logw(PyObject* self, PyObject* args);
static PyObject* ark_getAsset(PyObject* self, PyObject* args);
static PyObject* ark_getAssetResource(PyObject* self, PyObject* args);
static PyObject* ark_isDirectory(PyObject* self, PyObject* args);
static PyObject* ark_isFile(PyObject* self, PyObject* args);
static PyObject* ark_getRefManager(PyObject* self, PyObject* args);
static PyObject* ark_dirSeparator(PyObject* self, PyObject* args);
static PyObject* ark_trace_(PyObject* self, PyObject* args);

static PyMethodDef ARK_METHODS[] = {
    {"logd",  ark_logd, METH_VARARGS, "LOG_DEBUG"},
    {"logw",  ark_logw, METH_VARARGS, "LOG_WARN"},
    {"get_asset",  ark_getAsset, METH_VARARGS, "getAsset"},
    {"get_asset_resource",  ark_getAssetResource, METH_VARARGS, "getAssetResource"},
    {"is_directory",  ark_isDirectory, METH_VARARGS, "isDirectory"},
    {"is_file",  ark_isFile, METH_VARARGS, "isFile"},
    {"dir_separator",  ark_dirSeparator, METH_VARARGS, "dir_separator"},
    {"get_ref_manager",  ark_getRefManager, METH_VARARGS, "get_ref_manager"},
    {"__trace__",  ark_trace_, METH_VARARGS, "__trace__"},
    {nullptr, nullptr, 0, nullptr}
};

PyObject* ark_log(Log::LogLevel level, PyObject* args)
{
    size_t size = PyTuple_Size(args);
    if(size)
    {
        PyInstance pyContent = PyInstance::borrow(PyTuple_GetItem(args, 0));
        DASSERT(pyContent);
        PyInstance varargs = PyInstance::steal(PyTuple_GetSlice(args, 1, size));
        PyInstance formatted = PyInstance::steal(size > 1 ? PyUnicode_Format(pyContent, varargs) : PyObject_Str(pyContent));
        DCHECK(formatted, "Unsatisfied format: %s", PythonInterpreter::instance()->toString(pyContent).c_str());
        const String content = PythonInterpreter::instance()->toString(formatted);
        Log::log(level, "Python", content.c_str());
    }
    Py_RETURN_NONE;
}

PyObject* ark_logd(PyObject* /*self*/, PyObject* args)
{
    return ark_log(Log::LOG_LEVEL_DEBUG, args);
}

PyObject* ark_logw(PyObject* /*self*/, PyObject* args)
{
    return ark_log(Log::LOG_LEVEL_WARNING, args);
}

PyObject* ark_getAsset(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    if(!PyArg_ParseTuple(args, "s", &arg0))
        Py_RETURN_NONE;
    const sp<Readable> readable = Ark::instance().getResource(arg0);
    if(readable)
        return PythonInterpreter::instance()->fromType<String>(Strings::loadFromReadable(readable));
    Py_RETURN_NONE;
}

PyObject* ark_getAssetResource(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    if(!PyArg_ParseTuple(args, "s", &arg0))
        Py_RETURN_NONE;
    const sp<Asset> resource = Ark::instance().getAsset(*arg0 ? arg0 : "/");
    if(resource)
        return PythonInterpreter::instance()->template fromSharedPtr<AssetResource>(sp<AssetResource>::make(resource));
    Py_RETURN_NONE;
}

PyObject* ark_isDirectory(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    if(!PyArg_ParseTuple(args, "s", &arg0))
        Py_RETURN_FALSE;
    return PythonInterpreter::instance()->fromType<bool>(Platform::isDirectory(arg0));
}

PyObject* ark_isFile(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    if(!PyArg_ParseTuple(args, "s", &arg0))
        Py_RETURN_FALSE;
    return PythonInterpreter::instance()->fromType<bool>(Platform::isFile(arg0));
}

PyObject* ark_dirSeparator(PyObject* /*self*/, PyObject* /*args*/)
{
    return PythonInterpreter::instance()->fromType<String>(String(1, Platform::dirSeparator()));
}

PyObject* ark_getRefManager(PyObject* /*self*/, PyObject* /*args*/)
{
    return PythonInterpreter::instance()->toPyObject(PythonInterpreter::instance()->referenceManager());
}

PyObject* ark_trace_(PyObject* /*self*/, PyObject* /*args*/)
{
    __trace__();
    Py_RETURN_NONE;
}

PyObject* initarkmodule()
{
    static struct PyModuleDef cModPyArk = {
        PyModuleDef_HEAD_INIT,
        "ark",          /* name of module */
        "ark module",   /* module documentation, may be NULL */
        -1,             /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
        ARK_METHODS
    };

    PyObject* module = PyModule_Create(&cModPyArk);
    PythonInterpreter::newInstance();
    __init_py_ark_bindings__(module);
    return module;
}

static List<String> _PYTHON_PATH;

void setPythonPath(const List<String>& paths)
{
    _PYTHON_PATH = paths;
}

}
}
}

PyMODINIT_FUNC PyInit_ark(void)
{
    PyObject* module = ark::plugin::python::initarkmodule();
    PyObject* path = PyList_New(ark::plugin::python::_PYTHON_PATH.size());
    Py_ssize_t i = 0;
    for(const String& str : ark::plugin::python::_PYTHON_PATH)
        PyList_SetItem(path, i++, PyUnicode_FromString(str.c_str()));
    PyObject_SetAttrString(module, "path", path);
    return module;
}



