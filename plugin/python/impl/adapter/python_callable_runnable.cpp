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
    : _callable(std::move(callable))
{
}

void PythonCallableRunnable::run()
{
    DCHECK_THREAD_FLAG();

    CHECK_WARN(_callable, "This PyObject has been recycled by Python garbage collector");
    if(_callable.pyObject())
    {
        PyInstance args(PyInstance::steal(PyTuple_New(0)));
        PyObject* ret = _callable.call(args.pyObject());
        if(ret)
            Py_DECREF(ret);
        else
            PythonInterpreter::instance()->logErr();
    }
}

void PythonCallableRunnable::traverse(const Holder::Visitor& visitor)
{
    visitor(_callable.ref());
}

}
}
}
