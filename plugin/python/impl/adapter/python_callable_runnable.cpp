#include "python/impl/adapter/python_callable_runnable.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/log.h"

#include "python/api.h"
#include "python/extension/python_interpreter.h"
#include "python/extension/py_instance_ref.h"

namespace ark {
namespace plugin {
namespace python {

PythonCallableRunnable::PythonCallableRunnable(PyInstance callable)
    : Disposed(sp<Result>::make()), _callable(std::move(callable)), _result(_disposed->delegate())
{
}

void PythonCallableRunnable::run()
{
    DCHECK_THREAD_FLAG();

    DWARN(_callable, "This PyObject has been recycled by Python garbage collector");
    if(_callable.instance())
    {
        PyInstance args(PyInstance::steal(PyTuple_New(0)));
        PyObject* ret = _callable.call(args);
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
}

void PythonCallableRunnable::traverse(const Holder::Visitor& visitor)
{
    visitor(_callable.ref());
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
