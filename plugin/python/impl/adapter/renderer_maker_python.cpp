#include "python/impl/adapter/renderer_maker_python.h"

#include "python/extension/python_interpreter.h"

#include "python/api.h"

namespace ark {
namespace plugin {
namespace python {

RendererMakerPython::RendererMakerPython(PyInstance maker)
    : _maker(std::move(maker))
{
}

RendererMakerPython::RendererMakerPython(PyInstance maker, PyInstance recycler)
    : _maker(std::move(maker)), _recycler(std::move(recycler))
{
}

sp<Renderer> RendererMakerPython::make(int32_t x, int32_t y)
{
    DCHECK_THREAD_FLAG();

    PyInstance args(PyInstance::steal(PyTuple_New(2)));
    PyObject* pyX = PythonInterpreter::instance()->fromType<int32_t>(x);
    PyObject* pyY = PythonInterpreter::instance()->fromType<int32_t>(y);
    PyTuple_SetItem(args.pyObject(), 0, pyX);
    PyTuple_SetItem(args.pyObject(), 1, pyY);
    PyObject* ret = _maker.call(args.pyObject());
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

void RendererMakerPython::recycle(const sp<Renderer>& renderer)
{
    DCHECK_THREAD_FLAG();
    if(_recycler)
    {
        PyInstance args(PyInstance::steal(PyTuple_New(1)));
        PyTuple_SetItem(args.pyObject(), 0, PythonInterpreter::instance()->toPyObject(renderer));
        PyObject* ret = _recycler.call(args.pyObject());
        if(ret)
            Py_DECREF(ret);
        else
            PythonInterpreter::instance()->logErr();
    }
}

void RendererMakerPython::traverse(const Holder::Visitor& visitor)
{
    visitor(_maker.ref());
    if(_recycler)
        visitor(_recycler.ref());
}

}
}
}
