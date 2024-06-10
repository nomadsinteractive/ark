#include "python/impl/duck/py_callable_duck_type.h"

#include "core/base/observer.h"

#include "graphics/base/glyph.h"
#include "graphics/inf/glyph_maker.h"

#include "python/impl/adapter/python_callable_runnable.h"
#include "python/impl/adapter/python_callable_event_listener.h"
#include "python/impl/adapter/renderer_maker_python.h"

#include "python/extension/py_cast.h"

namespace ark::plugin::python {

namespace {

class PythonCallableCharacterMaker : public GlyphMaker {
public:
    PythonCallableCharacterMaker(PyInstance callable)
        : _callable(std::move(callable)) {
    }

    std::vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override {
        DCHECK_THREAD_FLAG();

        PyInstance args(PyInstance::steal(PyTuple_New(1)));
        PyTuple_SetItem(args.pyObject(), 0, PyCast::toPyObject(text));
        PyObject* ret = _callable.call(args.pyObject());
        if(ret)
        {
            const std::vector<sp<Glyph>> glyphs = ret == Py_None ? std::vector<sp<Glyph>>() : PyCast::ensureCppObject<std::vector<sp<Glyph>>>(ret);
            Py_DECREF(ret);
            return glyphs;
        }
        else
            PythonInterpreter::instance().logErr();

        return {};
    }

private:
    PyInstance _callable;
};

}


PyCallableDuckType::PyCallableDuckType(PyInstance inst)
    : _instance(std::move(inst))
{
}

void PyCallableDuckType::to(sp<Runnable>& inst)
{
    inst = sp<PythonCallableRunnable>::make(_instance);
}

void PyCallableDuckType::to(sp<Observer>& inst)
{
    inst = sp<Observer>::make(sp<PythonCallableRunnable>::make(_instance));
}

void PyCallableDuckType::to(sp<EventListener>& inst)
{
    inst = sp<PythonCallableEventListener>::make(_instance);
}

void PyCallableDuckType::to(sp<GlyphMaker>& inst)
{
    inst = sp<PythonCallableCharacterMaker>::make(_instance);
}

void PyCallableDuckType::to(sp<RendererMaker>& inst)
{
    inst = sp<RendererMakerPython>::make(_instance);
}

}
