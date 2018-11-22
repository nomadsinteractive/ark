#include "python/impl/adapter/python_callable_runnable.h"

#include "core/impl/variable/variable_wrapper.h"

#include "python/api.h"
#include "python/extension/python_interpreter.h"

namespace ark {
namespace plugin {
namespace python {

PythonCallableRunnable::PythonCallableRunnable(const sp<PyInstance>& callable)
    : Lifecycle(sp<Result>::make()), _args(PyInstance::steal(PyTuple_New(0))), _callable(callable), _result(_disposed->delegate())
{
}

void PythonCallableRunnable::run()
{
    DCHECK_THREAD_FLAG();

    PyObject* ret = _callable->call(_args);
    if(ret)
    {
        if(ret == Py_None)
            _result->setFunctionResult(FUNCTION_RESULT_NONE);
        else
            _result->setFunctionResult(PyObject_IsTrue(ret) ? FUNCTION_RESULT_TRUE : FUNCTION_RESULT_FALSE);
        Py_DECREF(ret);
    }
    else
        PythonInterpreter::instance()->logErr();
}

PythonCallableRunnable::Result::Result()
    : _function_result(FUNCTION_RESULT_NONE)
{
}

bool PythonCallableRunnable::Result::val()
{
    DWARN(_function_result != FUNCTION_RESULT_NONE, "'None' returned, which is ambiguous. Better returning True or False instead.");
    return _function_result != FUNCTION_RESULT_TRUE;

}

void PythonCallableRunnable::Result::setFunctionResult(PythonCallableRunnable::FunctionResult functionResult)
{
    _function_result = functionResult;
}

}
}
}
