#include "core/forwarding.h"
#include "core/ark.h"
#include "core/impl/readable/file_readable.h"
#include "core/types/shared_ptr.h"
#include "core/util/log.h"
#include "core/util/strings.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/base/application_context.h"

#include "python/impl/script/python_script.h"
#include "python/extension/py_instance.h"
#include "python/extension/py_cast.h"
#include "python/extension/py_ark_type.h"

#include "platform/platform.h"

#include "generated/py_ark_bindings.h"
#include "renderer/base/render_engine.h"

using namespace ark;

namespace ark::plugin::python {

static PyObject* ark_log(Log::LogLevel level, PyObject* args);
static PyObject* ark_logd(PyObject* self, PyObject* args);
static PyObject* ark_logw(PyObject* self, PyObject* args);
static PyObject* ark_loge(PyObject* self, PyObject* args);
static PyObject* ark_loadAsset(PyObject* self, PyObject* args);
static PyObject* ark_openAsset(PyObject* self, PyObject* args);
static PyObject* ark_loadAssetBundle(PyObject* self, PyObject* args);
static PyObject* ark_buildType(PyObject* self, PyObject* args);
static PyObject* ark_isDirectory(PyObject* self, PyObject* args);
static PyObject* ark_isFile(PyObject* self, PyObject* args);
static PyObject* ark_loadFile(PyObject* self, PyObject* args);
static PyObject* ark_getRefManager(PyObject* self, PyObject* args);
static PyObject* ark_dirSeparator(PyObject* self, PyObject* args);
static PyObject* ark_is_NDC_Y_Up(PyObject* self, PyObject* args);
static PyObject* ark_trace_(PyObject* self, PyObject* args);

static int __traverse__(PyObject* module, visitproc visitor, void* args)
{
    return 0;
}

static PyMethodDef ARK_METHODS[] = {
    {"logd",  ark_logd, METH_VARARGS, "logd"},
    {"logw",  ark_logw, METH_VARARGS, "logw"},
    {"loge",  ark_loge, METH_VARARGS, "loge"},
    {"load_asset",  ark_loadAsset, METH_VARARGS, "loadAsset"},
    {"open_asset",  ark_openAsset, METH_VARARGS, "openAsset"},
    {"load_asset_bundle",  ark_loadAssetBundle, METH_VARARGS, "loadAssetBundle"},
    {"build_type",  ark_buildType, METH_VARARGS, "build_type"},
    {"is_directory",  ark_isDirectory, METH_VARARGS, "isDirectory"},
    {"is_file",  ark_isFile, METH_VARARGS, "isFile"},
    {"load_file",  ark_loadFile, METH_VARARGS, "loadFile"},
    {"dir_separator",  ark_dirSeparator, METH_VARARGS, "dir_separator"},
    {"is_ndc_y_up",  ark_is_NDC_Y_Up, METH_VARARGS, "is_ndc_y_up"},
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
        if(pyContent.isNone())
            Log::log(level, "Python", "None");
        else
        {
            PyInstance varargs = PyInstance::steal(PyTuple_GetSlice(args, 1, size));
            PyInstance formatted = PyInstance::steal(size > 1 ? PyUnicode_Format(pyContent.pyObject(), varargs.pyObject()) : PyObject_Str(pyContent.pyObject()));
            CHECK(formatted, "Unsatisfied format: %s", PyCast::toString(pyContent.pyObject()).c_str());
            const String content = PyCast::toString(formatted.pyObject());
            Log::log(level, "Python", content.c_str());
        }
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

PyObject* ark_loge(PyObject* /*self*/, PyObject* args)
{
    return ark_log(Log::LOG_LEVEL_ERROR, args);
}

PyObject* ark_loadAsset(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    if(!PyArg_ParseTuple(args, "s", &arg0))
        Py_RETURN_NONE;
    if(const sp<Readable> readable = Ark::instance().tryOpenAsset(arg0))
        return PyCast::toPyObject_impl<String>(Strings::loadFromReadable(readable));
    Py_RETURN_NONE;
}

PyObject* ark_openAsset(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    if(!PyArg_ParseTuple(args, "s", &arg0))
        Py_RETURN_NONE;
    const sp<Readable> readable = Ark::instance().tryOpenAsset(arg0);
    return PyCast::toPyObject(readable);
}

PyObject* ark_loadAssetBundle(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    if(!PyArg_ParseTuple(args, "s", &arg0))
        Py_RETURN_NONE;
    return PyCast::toPyObject(Ark::instance().getAssetBundle(*arg0 ? arg0 : "/"));
}

PyObject* ark_isDirectory(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    if(!PyArg_ParseTuple(args, "s", &arg0))
        Py_RETURN_FALSE;
    return PyCast::toPyObject_impl<bool>(Platform::isDirectory(arg0));
}

PyObject* ark_buildType(PyObject* /*self*/, PyObject* /*args*/)
{
#ifdef ARK_FLAG_DEBUG
    return PyCast::toPyObject(static_cast<Enum::BuildType>(ARK_FLAG_DEBUG));
#else
    return PyCast::toPyObject(Enum::BUILD_TYPE_UNDEFINED);
#endif
}

PyObject* ark_isFile(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    if(!PyArg_ParseTuple(args, "s", &arg0))
        Py_RETURN_FALSE;
    return PyCast::toPyObject_impl<bool>(Platform::isFile(arg0));
}

PyObject* ark_loadFile(PyObject* /*self*/, PyObject* args)
{
    const char* arg0, *arg1;
    if(!PyArg_ParseTuple(args, "ss", &arg0, &arg1))
        Py_RETURN_NONE;
    const sp<Readable> readable = sp<FileReadable>::make(arg0, arg1);
    return PyCast::toPyObject_impl<String>(Strings::loadFromReadable(readable));
}

PyObject* ark_dirSeparator(PyObject* /*self*/, PyObject* /*args*/)
{
    const char ds[2] = {Platform::dirSeparator(), 0};
    return PyCast::toPyObject_impl<String>(ds);
}

PyObject* ark_is_NDC_Y_Up(PyObject* /*self*/, PyObject* /*args*/)
{
    if(Ark::instance().renderController()->renderEngine()->isYUp())
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

PyObject* ark_getRefManager(PyObject* /*self*/, PyObject* /*args*/)
{
    return PyCast::toPyObject(PythonInterpreter::instance().referenceManager());
}

PyObject* ark_trace_(PyObject* /*self*/, PyObject* /*args*/)
{
    TRACE(true, "");
    Py_RETURN_NONE;
}

PyObject* initarkmodule()
{
    static struct PyModuleDef cModPyArk = {
        PyModuleDef_HEAD_INIT,
        "ark",          /* name of module */
        "ark module",   /* module documentation, may be NULL */
        -1,             /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
        ARK_METHODS,
        nullptr,
        __traverse__,
        nullptr,
        nullptr
    };

    PyObject* module = PyModule_Create(&cModPyArk);
    __init_py_ark_bindings__(module);
    return module;
}

}

PyMODINIT_FUNC PyInit_ark(void)
{
    const std::vector<String>& paths = static_cast<ark::plugin::python::PythonScript*>(Ark::instance().applicationContext()->interpreter().get())->paths();
    PyObject* module = ark::plugin::python::initarkmodule();
    PyObject* path = PyList_New(static_cast<Py_ssize_t>(paths.size()));
    Py_ssize_t i = 0;
    for(const String& str : paths)
        PyList_SetItem(path, i++, PyUnicode_FromString(str.c_str()));
    PyObject_SetAttrString(module, "path", path);
    Py_DECREF(path);
    return module;
}



