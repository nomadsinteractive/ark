#include "python/impl/duck/py_callable_duck_type.h"

#include "graphics/inf/character_maker.h"
#include "graphics/inf/character_mapper.h"

#include "python/impl/adapter/python_callable_runnable.h"
#include "python/impl/adapter/python_callable_event_listener.h"
#include "python/impl/adapter/python_callable_tile_maker.h"

#include "python/extension/python_interpreter.h"

namespace ark {
namespace plugin {
namespace python {

namespace {

class PythonCallableCharacterMaker : public CharacterMaker {
public:
    PythonCallableCharacterMaker(const sp<PyInstance>& callable)
        : _callable(callable), _args(PyInstance::steal(PyTuple_New(3))) {
    }

    virtual sp<RenderObject> makeCharacter(int32_t type, const sp<Vec>& position, const sp<Size>& size) override {
        DCHECK_THREAD_FLAG();

        const sp<PythonInterpreter>& interpreter = PythonInterpreter::instance();
        PyObject* pyType = interpreter->toPyObject<int32_t>(type);
        PyObject* pyPosition = interpreter->toPyObject<sp<Vec>>(position);
        PyObject* pySize = interpreter->toPyObject<sp<Size>>(size);
        PyTuple_SetItem(_args, 0, pyType);
        PyTuple_SetItem(_args, 1, pyPosition);
        PyTuple_SetItem(_args, 2, pySize);
        PyObject* ret = _callable->call(_args);
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
    sp<PyInstance> _callable;
    PyInstance _args;

};

class PythonCallableCharacterMapper : public CharacterMapper {
public:
    PythonCallableCharacterMapper(const sp<PyInstance>& callable)
        : _callable(callable), _args(PyInstance::steal(PyTuple_New(1))) {
    }

    virtual int32_t mapCharacter(int32_t c) override {
        DCHECK_THREAD_FLAG();

        const sp<PythonInterpreter>& interpreter = PythonInterpreter::instance();
        PyObject* pyType = interpreter->toPyObject<int32_t>(c);
        PyTuple_SetItem(_args, 0, pyType);
        PyInstance ret = PyInstance::steal(_callable->call(_args));
        if(ret)
            return PythonInterpreter::instance()->toCppObject<int32_t>(ret);

        PythonInterpreter::instance()->logErr();
        return 0;
    }

private:
    sp<PyInstance> _callable;
    PyInstance _args;

};

}


PyCallableDuckType::PyCallableDuckType(const sp<PyInstance>& inst)
    : _instance(inst)
{
}

void PyCallableDuckType::to(sp<Runnable>& inst)
{
    inst = sp<PythonCallableRunnable>::make(PyInstance::track(_instance->object()));
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

void PyCallableDuckType::to(sp<TileMaker>& inst)
{
    inst = sp<PythonCallableTileMaker>::make(_instance);
}

}
}
}
