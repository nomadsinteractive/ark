#include "python/impl/adapter/python_callable_runnable.h"

#include "core/util/log.h"

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
        _expired = _not_none_returned ? PyObject_IsTrue(ret) == 0 : false;
        Py_DECREF(ret);
    }
    else
        PythonInterpreter::instance()->logErr();
}

bool PythonCallableRunnable::val()
{
    DWARN(_not_none_returned, "Expirable Runnable returns 'None', which has an ambiguous meaning. Better returning True or False instead.");
    return _expired;
}

}
}
}
