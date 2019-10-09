#include "python/impl/duck/py_callable_duck_type.h"

#include "core/base/observer.h"

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
        : _callable(std::move(callable)), _args(PyInstance::steal(PyTuple_New(3))) {
    }

    virtual sp<RenderObject> makeCharacter(int32_t type, const V2& position, const sp<Size>& size) override {
        DCHECK_THREAD_FLAG();

        const sp<PythonInterpreter>& interpreter = PythonInterpreter::instance();
        PyObject* pyType = interpreter->toPyObject<int32_t>(type);
        PyObject* pyPosition = interpreter->toPyObject<V2>(position);
        PyObject* pySize = interpreter->toPyObject<sp<Size>>(size);
        PyTuple_SetItem(_args.pyObject(), 0, pyType);
        PyTuple_SetItem(_args.pyObject(), 1, pyPosition);
        PyTuple_SetItem(_args.pyObject(), 2, pySize);
        PyObject* ret = _callable.call(_args.pyObject());
        if(ret)
        {
            const sp<RenderObject> renderObject = ret == Py_None ? nullptr : PythonInterpreter::instance()->toCppObject<sp<RenderObject>>(ret);
            Py_DECREF(ret);
            return renderObject;
        }
        else
            PythonInterpreter::instance()->logErr();

        return nullptr;
    }

private:
    PyInstance _callable;
    PyInstance _args;

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
