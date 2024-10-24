#include "python/impl/adapter/runnable_python.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/log.h"

#include "python/api.h"
#include "python/extension/python_extension.h"

namespace ark::plugin::python {

RunnablePython::RunnablePython(PyInstance callable)
    : _callable(std::move(callable))
{
}

void RunnablePython::run()
{
    DCHECK_THREAD_FLAG();

    CHECK_WARN(_callable, "This PyObject has been recycled by Python garbage collector");
    if(_callable.pyObject())
    {
        const PyInstance args(PyInstance::steal(PyTuple_New(0)));
        if(PyObject* ret = _callable.call(args.pyObject()))
            Py_DECREF(ret);
        else
            PythonExtension::instance().logErr();
    }
}

void RunnablePython::traverse(const Visitor& visitor)
{
    visitor(_callable.ref());
}

}
