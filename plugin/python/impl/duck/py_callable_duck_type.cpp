#include "python/impl/duck/py_callable_duck_type.h"

#include "core/base/observer.h"

#include "graphics/base/glyph.h"
#include "graphics/inf/character_maker.h"
#include "graphics/inf/character_mapper.h"

#include "python/impl/adapter/python_callable_runnable.h"
#include "python/impl/adapter/python_callable_event_listener.h"
#include "python/impl/adapter/renderer_maker_python.h"

#include "python/extension/python_interpreter.h"

namespace ark {
namespace plugin {
namespace python {

namespace {

class PythonCallableCharacterMaker : public CharacterMaker {
public:
    PythonCallableCharacterMaker(PyInstance callable)
        : _callable(std::move(callable)) {
    }

    virtual std::vector<sp<RenderObject>> makeCharacter(const std::vector<Glyph>& glyphs) override {
        DCHECK_THREAD_FLAG();

        const sp<PythonInterpreter>& interpreter = PythonInterpreter::instance();
        PyInstance args(PyInstance::steal(PyTuple_New(1)));

        PyObject* pyArg = PyList_New(static_cast<Py_ssize_t>(glyphs.size()));
        for(size_t i = 0; i < glyphs.size(); ++i)
            PyList_SetItem(pyArg, static_cast<Py_ssize_t>(i), interpreter->toPyObject(sp<Glyph>::make(glyphs.at(i))));
        PyTuple_SetItem(args.pyObject(), 0, pyArg);
        PyObject* ret = _callable.call(args.pyObject());
        if(ret)
        {
            const std::vector<sp<RenderObject>> renderObjects = ret == Py_None ? std::vector<sp<RenderObject>>() : PythonInterpreter::instance()->toCppObject<std::vector<sp<RenderObject>>>(ret);
            Py_DECREF(ret);
            return renderObjects;
        }
        else
            PythonInterpreter::instance()->logErr();

        return {};
    }

    virtual V2 scale() override {
        return V2(1.0f);
    }

private:
    PyInstance _callable;
};

class PythonCallableCharacterMapper : public CharacterMapper {
public:
    PythonCallableCharacterMapper(PyInstance callable)
        : _callable(std::move(callable)), _args(PyInstance::steal(PyTuple_New(1))) {
    }

    virtual int32_t mapCharacter(int32_t c) override {
        DCHECK_THREAD_FLAG();

        const sp<PythonInterpreter>& interpreter = PythonInterpreter::instance();
        PyObject* pyType = interpreter->toPyObject<int32_t>(c);
        PyTuple_SetItem(_args.pyObject(), 0, pyType);
        PyInstance ret = PyInstance::steal(_callable.call(_args.pyObject()));
        if(ret)
            return PythonInterpreter::instance()->toCppObject<int32_t>(ret.pyObject());

        PythonInterpreter::instance()->logErr();
        return 0;
    }

private:
    PyInstance _callable;
    PyInstance _args;

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

void PyCallableDuckType::to(sp<CharacterMaker>& inst)
{
    inst = sp<PythonCallableCharacterMaker>::make(_instance);
}

void PyCallableDuckType::to(sp<CharacterMapper>& inst)
{
    inst = sp<PythonCallableCharacterMapper>::make(_instance);
}

void PyCallableDuckType::to(sp<RendererMaker>& inst)
{
    inst = sp<RendererMakerPython>::make(_instance);
}

}
}
}
