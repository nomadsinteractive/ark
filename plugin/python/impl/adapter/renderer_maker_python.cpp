#include "python/impl/adapter/renderer_maker_python.h"

#include "python/extension/py_cast.h"

#include "python/api.h"

namespace ark::plugin::python {

RendererMakerPython::RendererMakerPython(PyInstance maker)
    : _maker(std::move(maker))
{
}

RendererMakerPython::RendererMakerPython(PyInstance maker, PyInstance recycler)
    : _maker(std::move(maker)), _recycler(std::move(recycler))
{
}

std::vector<Box> RendererMakerPython::make(float x, float y)
{
    DCHECK_THREAD_FLAG();

    PyInstance args(PyInstance::steal(PyTuple_New(2)));
    PyObject* pyX = PyCast::toPyObject<float>(x);
    PyObject* pyY = PyCast::toPyObject<float>(y);
    PyTuple_SetItem(args.pyObject(), 0, pyX);
    PyTuple_SetItem(args.pyObject(), 1, pyY);
    PyInstance ret = PyInstance::steal(_maker.call(args.pyObject()));
    if(!ret.isNullptr())
    {
        if(ret.isList())
            return PyCast::ensureCppObject<std::vector<Box>>(ret.pyObject());

        return {PyCast::ensureSharedPtr<Box>(ret.pyObject())};
    }
    else
        PythonInterpreter::instance().logErr();

    return {};
}

void RendererMakerPython::recycle(const Box& renderer)
{
    DCHECK_THREAD_FLAG();
    if(_recycler)
    {
        PyInstance args(PyInstance::steal(PyTuple_New(1)));
        PyTuple_SetItem(args.pyObject(), 0, PythonInterpreter::instance().toPyObject(renderer));
        PyObject* ret = _recycler.call(args.pyObject());
        if(ret)
            Py_DECREF(ret);
        else
            PythonInterpreter::instance().logErr();
    }
}

}
