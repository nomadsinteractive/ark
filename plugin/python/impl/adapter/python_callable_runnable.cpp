#include "python/impl/adapter/python_callable_runnable.h"

#include "core/impl/variable/variable_wrapper.h"

#include "python/api.h"
#include "python/extension/python_interpreter.h"

namespace ark {
namespace plugin {
namespace python {

PythonCallableRunnable::PythonCallableRunnable(const sp<PyInstance>& callable)
    : Lifecycle(false), _args(PyInstance::steal(PyTuple_New(0))), _callable(callable), _not_none_returned(true)
{
}

void PythonCallableRunnable::run()
{
    DCHECK_THREAD_FLAG();

    PyObject* ret = _callable->call(_args);
    if(ret)
    {
        _not_none_returned = ret != Py_None;
        _disposed->set(_not_none_returned ? PyObject_IsTrue(ret) == 0 : false);
        Py_DECREF(ret);
    }
    else
        PythonInterpreter::instance()->logErr();
}

bool PythonCallableRunnable::val()
{
    DWARN(_not_none_returned, "'None' returned, which is ambiguous. Better returning True or False instead.");
    return _disposed->val();
}

}
}
}
