#include "python/impl/duck/py_callable_duck_type.h"

#include "core/base/observer.h"

#include "graphics/base/glyph.h"
#include "graphics/inf/glyph_maker.h"

#include "python/impl/adapter/python_callable_runnable.h"
#include "python/impl/adapter/python_callable_event_listener.h"
#include "python/impl/adapter/renderer_maker_python.h"

#include "python/extension/python_interpreter.h"

namespace ark {
namespace plugin {
namespace python {

namespace {

class PythonCallableCharacterMaker : public GlyphMaker {
public:
    PythonCallableCharacterMaker(PyInstance callable)
        : _callable(std::move(callable)) {
    }

    virtual std::vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override {
        DCHECK_THREAD_FLAG();

        const sp<PythonInterpreter>& interpreter = PythonInterpreter::instance();
        PyInstance args(PyInstance::steal(PyTuple_New(1)));
        PyTuple_SetItem(args.pyObject(), 0, interpreter->toPyObject(text));
        PyObject* ret = _callable.call(args.pyObject());
        if(ret)
        {
            const std::vector<sp<Glyph>> glyphs = ret == Py_None ? std::vector<sp<Glyph>>() : PythonInterpreter::instance()->toCppObject<std::vector<sp<Glyph>>>(ret);
            Py_DECREF(ret);
            return glyphs;
        }
        else
            PythonInterpreter::instance()->logErr();

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
}
}
