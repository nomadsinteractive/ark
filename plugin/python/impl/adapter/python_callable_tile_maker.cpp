#include "python/impl/adapter/python_callable_tile_maker.h"

#include "python/extension/python_interpreter.h"

#include "python/api.h"

namespace ark {
namespace plugin {
namespace python {

PythonCallableTileMaker::PythonCallableTileMaker(PyInstance callable)
    : _callable(std::move(callable)), _args(PyInstance::steal(PyTuple_New(2)))
{
}

sp<Renderer> PythonCallableTileMaker::make(int32_t x, int32_t y)
{
    DCHECK_THREAD_FLAG();

    PyObject* pyX = PythonInterpreter::instance()->fromType<int32_t>(x);
    PyObject* pyY = PythonInterpreter::instance()->fromType<int32_t>(y);
    PyTuple_SetItem(_args, 0, pyX);
    PyTuple_SetItem(_args, 1, pyY);
    PyObject* ret = _callable.call(_args);
    if(ret)
    {
        const sp<Renderer> renderer = PythonInterpreter::instance()->toSharedPtr<Renderer>(ret);
        Py_DECREF(ret);
        return renderer;
    }
    else
        PythonInterpreter::instance()->logErr();

    return nullptr;
}

}
}
}
