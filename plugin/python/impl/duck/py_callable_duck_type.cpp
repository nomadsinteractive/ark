#include "python/impl/duck/py_callable_duck_type.h"

#include "core/base/observer.h"

#include "graphics/base/glyph.h"
#include "graphics/inf/glyph_maker.h"

#include "python/impl/adapter/runnable_python.h"
#include "python/impl/adapter/event_listener_python.h"
#include "python/impl/adapter/renderer_maker_python.h"

#include "python/extension/py_cast.h"

namespace ark::plugin::python {

namespace {

class PythonCallableCharacterMaker final : public GlyphMaker {
public:
    PythonCallableCharacterMaker(PyInstance callable)
        : _callable(std::move(callable)) {
    }

    std::vector<sp<Glyph>> makeGlyphs(const std::wstring& text) override {
        DCHECK_THREAD_FLAG();

        const PyInstance args(PyInstance::steal(PyTuple_New(1)));
        PyTuple_SetItem(args.pyObject(), 0, PyCast::toPyObject(text));
        if(PyObject* ret = _callable.call(args.pyObject()))
        {
            const std::vector<sp<Glyph>> glyphs = ret == Py_None ? std::vector<sp<Glyph>>() : PyCast::ensureCppObject<std::vector<sp<Glyph>>>(ret);
            Py_DECREF(ret);
            return glyphs;
        }

        PythonExtension::instance().logErr();
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
    WARN("Converting python function to Runnable directly is deprecated. Use Behavior class as an alternative.");
    inst = sp<RunnablePython>::make(_instance);
}

void PyCallableDuckType::to(sp<EventListener>& inst)
{
    inst = sp<EventListenerPython>::make(_instance);
}

void PyCallableDuckType::to(sp<GlyphMaker>& inst)
{
    inst = sp<GlyphMaker>::make<PythonCallableCharacterMaker>(_instance);
}

void PyCallableDuckType::to(sp<RendererMaker>& inst)
{
    inst = sp<RendererMakerPython>::make(_instance);
}

}
